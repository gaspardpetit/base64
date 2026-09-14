/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 Gaspard Petit
 */

#if defined(BASE64_ENABLE_AVX2) && defined(_MSC_VER) && defined(_M_X64)

#define base64_encode base64_scalar_encode
#define base64url_encode base64url_scalar_encode
#define base64_decode base64_scalar_decode
#define base64_decode_unchecked base64_scalar_decode_unchecked
#define BASE64_IMPLEMENTATION
#include "base64.h"
#undef base64_encode
#undef base64url_encode
#undef base64_decode
#undef base64_decode_unchecked

#include "base64_avx2.h"
#include <intrin.h>
#include <windows.h>

static int base64_has_avx2(void)
{
    static volatile LONG cached = 0;
    LONG value = InterlockedCompareExchange(&cached, 0, 0);
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
    }
    return value == 2;
}

#ifdef __cplusplus
extern "C" {
#endif

size_t base64_encode(const unsigned char* input, size_t length, char* output)
{
    return base64_has_avx2() && length >= 100U
        ? base64_avx2_encode(input, length, output)
        : base64_scalar_encode(input, length, output);
}

size_t base64url_encode(const unsigned char* input, size_t length, char* output)
{
    return base64_has_avx2() && length >= 100U
        ? base64url_avx2_encode(input, length, output)
        : base64url_scalar_encode(input, length, output);
}

size_t base64_decode(const unsigned char* input, size_t length,
                     unsigned char* output)
{
    return base64_has_avx2() && length >= 104U
        ? base64_avx2_decode(input, length, output)
        : base64_scalar_decode(input, length, output);
}

size_t base64_decode_unchecked(const unsigned char* input, size_t length,
                               unsigned char* output)
{
    return base64_has_avx2() && length >= 104U
        ? base64_avx2_decode_unchecked(input, length, output)
        : base64_scalar_decode_unchecked(input, length, output);
}

#ifdef __cplusplus
}
#endif

#else

#define BASE64_IMPLEMENTATION
#include "base64.h"

#endif
