/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 Gaspard Petit
 */

#if ((defined(_MSC_VER) && defined(_M_X64)) || \
     ((defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__))) && \
    !defined(BASE64_DISABLE_HARDWARE)

#pragma push_macro("base64_encode")
#pragma push_macro("base64_decode")
#pragma push_macro("base64_decode_unchecked")
#pragma push_macro("base64_compact")
#undef base64_encode
#undef base64_decode
#undef base64_decode_unchecked
#undef base64_compact
#define base64_encode base64_scalar_encode
#define base64url_encode base64url_scalar_encode
#define base64_decode base64_scalar_decode
#define base64_decode_unchecked base64_scalar_decode_unchecked
#define base64_compact base64_scalar_compact
#define BASE64_IMPLEMENTATION
#include "base64.h"
#undef base64_encode
#undef base64url_encode
#undef base64_decode
#undef base64_decode_unchecked
#undef base64_compact
#pragma pop_macro("base64_compact")
#pragma pop_macro("base64_decode_unchecked")
#pragma pop_macro("base64_decode")
#pragma pop_macro("base64_encode")

#include "base64_avx2.h"
#if defined(_MSC_VER)
#include <intrin.h>
#include <windows.h>

static int base64_has_avx2(void)
{
    static volatile LONG cached = 0;
    LONG value = cached;
    if (value == 0) {
        int registers[4];
        int supported = 0;
        __cpuid(registers, 0);
        if (registers[0] >= 7) {
            __cpuid(registers, 1);
            if ((registers[2] & (1 << 27)) != 0 &&
                (registers[2] & (1 << 28)) != 0 &&
                (_xgetbv(0) & 6) == 6) {
                __cpuidex(registers, 7, 0);
                supported = (registers[1] & (1 << 5)) != 0;
            }
        }
        value = supported ? 2 : 1;
        InterlockedCompareExchange(&cached, value, 0);
        value = cached;
    }
    return value == 2;
}

#else
static int base64_has_avx2(void)
{
    /* Compiler CPU detection also checks OS support for AVX state. */
    return __builtin_cpu_supports("avx2") != 0;
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

size_t base64_encode(const unsigned char* input, size_t length, char* output)
{
    if (length >= 16U && base64_has_avx2())
        return length < 28U ? base64_avx128_encode(input, length, output)
                            : base64_avx2_encode(input, length, output);
    return base64_scalar_encode(input, length, output);
}

size_t base64url_encode(const unsigned char* input, size_t length, char* output)
{
    if (length >= 16U && base64_has_avx2())
        return length < 28U ? base64url_avx128_encode(input, length, output)
                            : base64url_avx2_encode(input, length, output);
    return base64url_scalar_encode(input, length, output);
}

size_t base64_decode(const unsigned char* input, size_t length,
                     unsigned char* output, int support_url_safe)
{
    if (length >= 24U && base64_has_avx2())
        return support_url_safe
            ? base64_avx2_decode(input, length, output)
            : base64_avx2_decode_standard(input, length, output);
    return base64_scalar_decode(input, length, output, support_url_safe);
}

size_t base64_decode_unchecked(const unsigned char* input, size_t length,
                               unsigned char* output, int support_url_safe)
{
    if (length >= 24U && base64_has_avx2())
        return support_url_safe
            ? base64_avx2_decode_unchecked(input, length, output)
            : base64_avx2_decode_standard_unchecked(input, length, output);
    return base64_scalar_decode_unchecked(input, length, output,
                                          support_url_safe);
}

size_t base64_compact(unsigned char* buffer, size_t length)
{
    if (length >= 32U && base64_has_avx2())
        return base64_avx2_compact(buffer, length);
    return base64_scalar_compact(buffer, length);
}

#ifdef __cplusplus
}
#endif

#elif defined(__aarch64__) && !defined(BASE64_DISABLE_HARDWARE)

#pragma push_macro("base64_encode")
#pragma push_macro("base64_decode")
#pragma push_macro("base64_decode_unchecked")
#pragma push_macro("base64_compact")
#undef base64_encode
#undef base64_decode
#undef base64_decode_unchecked
#undef base64_compact
#define base64_encode base64_scalar_encode
#define base64url_encode base64url_scalar_encode
#define base64_decode base64_scalar_decode
#define base64_decode_unchecked base64_scalar_decode_unchecked
#define base64_compact base64_scalar_compact
#define BASE64_IMPLEMENTATION
#include "base64.h"
#undef base64_encode
#undef base64url_encode
#undef base64_decode
#undef base64_decode_unchecked
#undef base64_compact
#pragma pop_macro("base64_compact")
#pragma pop_macro("base64_decode_unchecked")
#pragma pop_macro("base64_decode")
#pragma pop_macro("base64_encode")

#include "base64_neon.h"

static size_t base64_neon_encode_dispatch(const unsigned char* input,
                                          size_t length, char* output,
                                          int url_safe)
{
    const size_t vector_length = length & ~(size_t)47U;
    const size_t consumed = url_safe
        ? base64url_neon_encode_blocks(input, vector_length, output)
        : base64_neon_encode_blocks(input, vector_length, output);
    const size_t written = consumed / 3U * 4U;
    return written + (url_safe
        ? base64url_scalar_encode(input + consumed, length - consumed,
                                  output + written)
        : base64_scalar_encode(input + consumed, length - consumed,
                               output + written));
}

static size_t base64_neon_decode_dispatch(const unsigned char* input,
                                          size_t length,
                                          unsigned char* output,
                                          int unchecked,
                                          int support_url_safe)
{
    size_t vector_length = length & ~(size_t)63U;
    /* Keep the final padded quantum for the scalar frontend. */
    if (length != 0U && input[length - 1U] == '=')
        vector_length = (length - 4U) & ~(size_t)63U;
    if (vector_length != 0U) {
        if (unchecked) {
            /* Select the specialized URL-safe pipeline once.  The standard
             * loop remains free of URL-safe comparisons. */
            if (support_url_safe &&
                (memchr(input, '-', vector_length) != NULL ||
                 memchr(input, '_', vector_length) != NULL))
                base64url_neon_decode_blocks_unchecked(input, vector_length,
                                                        output);
            else
                base64_neon_decode_blocks_unchecked(input, vector_length, output);
        }
        else if (!base64_neon_decode_blocks(input, vector_length, output)) {
            /* Avoid a pre-scan on standard input.  A failed standard mapping
             * may be URL-safe, so retry with its dedicated SIMD pipeline;
             * scalar validation remains the fallback for invalid input. */
            if (!support_url_safe ||
                !base64url_neon_decode_blocks(input, vector_length, output))
                return base64_scalar_decode(input, length, output,
                                            support_url_safe);
        }
    }
    {
        const size_t tail = unchecked
            ? base64_scalar_decode_unchecked(input + vector_length,
                                             length - vector_length,
                                             output + vector_length / 4U * 3U,
                                             support_url_safe)
            : base64_scalar_decode(input + vector_length, length - vector_length,
                                   output + vector_length / 4U * 3U,
                                   support_url_safe);
        return tail == BASE64_ERROR ? BASE64_ERROR
                                    : vector_length / 4U * 3U + tail;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

size_t base64_encode(const unsigned char* input, size_t length, char* output)
{
    return length >= 48U ? base64_neon_encode_dispatch(input, length, output, 0)
                         : base64_scalar_encode(input, length, output);
}

size_t base64url_encode(const unsigned char* input, size_t length, char* output)
{
    return length >= 48U ? base64_neon_encode_dispatch(input, length, output, 1)
                         : base64url_scalar_encode(input, length, output);
}

size_t base64_decode(const unsigned char* input, size_t length,
                     unsigned char* output, int support_url_safe)
{
    return length >= 64U
        ? base64_neon_decode_dispatch(input, length, output, 0, support_url_safe)
        : base64_scalar_decode(input, length, output, support_url_safe);
}

size_t base64_decode_unchecked(const unsigned char* input, size_t length,
                               unsigned char* output, int support_url_safe)
{
    return length >= 64U
        ? base64_neon_decode_dispatch(input, length, output, 1, support_url_safe)
        : base64_scalar_decode_unchecked(input, length, output,
                                         support_url_safe);
}

size_t base64_compact(unsigned char* buffer, size_t length)
{
    return length >= 64U ? base64_neon_compact(buffer, length)
                         : base64_scalar_compact(buffer, length);
}

#ifdef __cplusplus
}
#endif

#else

#define BASE64_IMPLEMENTATION
#include "base64.h"

#endif

#ifdef __cplusplus
extern "C" {
#endif

size_t base64_decode_compiled(const unsigned char* input, size_t length,
                              unsigned char* output, int support_url_safe)
{
    return base64_decode(input, length, output, support_url_safe);
}

size_t base64_encode_compiled(const unsigned char* input, size_t length,
                              char* output)
{
    return base64_encode(input, length, output);
}

size_t base64_decode_unchecked_compiled(const unsigned char* input,
                                        size_t length,
                                        unsigned char* output,
                                        int support_url_safe)
{
    return base64_decode_unchecked(input, length, output, support_url_safe);
}

size_t base64_decode_whitespace(unsigned char* input, size_t length,
                                unsigned char* output, int support_url_safe)
{
    return base64_decode(input, base64_compact(input, length), output,
                         support_url_safe);
}

const char* base64_runtime_backend(void)
{
#if ((defined(_MSC_VER) && defined(_M_X64)) || \
     ((defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__))) && \
    !defined(BASE64_DISABLE_HARDWARE)
    return base64_has_avx2() ? "AVX2" : "scalar";
#elif defined(__aarch64__) && !defined(BASE64_DISABLE_HARDWARE)
    return "NEON";
#else
    return "scalar";
#endif
}

#ifdef __cplusplus
}
#endif
