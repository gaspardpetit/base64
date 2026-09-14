/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 Gaspard Petit
 */

#include "base64_avx2.h"
#include "base64.h"
#include <immintrin.h>
#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER)
#define BASE64_AVX2_INLINE __forceinline
#else
#define BASE64_AVX2_INLINE inline __attribute__((always_inline))
#endif

size_t base64_scalar_encode(const unsigned char*, size_t, char*);
size_t base64url_scalar_encode(const unsigned char*, size_t, char*);
size_t base64_scalar_decode(const unsigned char*, size_t, unsigned char*);
size_t base64_scalar_decode_unchecked(const unsigned char*, size_t,
                                      unsigned char*);

static BASE64_AVX2_INLINE __m256i encode_unpack(__m256i value)
{
    const __m256i a = _mm256_mulhi_epu16(
        _mm256_and_si256(value, _mm256_set1_epi32(0x0fc0fc00)),
        _mm256_set1_epi32(0x04000040));
    const __m256i b = _mm256_mullo_epi16(
        _mm256_and_si256(value, _mm256_set1_epi32(0x003f03f0)),
        _mm256_set1_epi32(0x01000010));
    return _mm256_or_si256(a, b);
}

static BASE64_AVX2_INLINE __m256i encode_load(const unsigned char* input)
{
    const __m128i lo = _mm_loadu_si128((const __m128i*)input);
    const __m128i hi = _mm_loadu_si128((const __m128i*)(input + 12));
    __m256i value = _mm256_inserti128_si256(
        _mm256_castsi128_si256(lo), hi, 1);
    return _mm256_shuffle_epi8(value, _mm256_setr_epi8(
        1,0,2,1,4,3,5,4,7,6,8,7,10,9,11,10,
        1,0,2,1,4,3,5,4,7,6,8,7,10,9,11,10));
}

static BASE64_AVX2_INLINE __m256i encode_translate(__m256i value, int url_safe)
{
    __m256i index = _mm256_subs_epu8(value, _mm256_set1_epi8(51));
    index = _mm256_sub_epi8(index,
        _mm256_cmpgt_epi8(value, _mm256_set1_epi8(25)));
    if (url_safe) {
        const __m256i offsets = _mm256_setr_epi8(
            65,71,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-17,32,0,0,
            65,71,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-17,32,0,0);
        return _mm256_add_epi8(value, _mm256_shuffle_epi8(offsets, index));
    }
    else {
        const __m256i offsets = _mm256_setr_epi8(
            65,71,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-19,-16,0,0,
            65,71,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-19,-16,0,0);
        return _mm256_add_epi8(value, _mm256_shuffle_epi8(offsets, index));
    }
}

#define BASE64_AVX2_ENCODE_BODY(url_safe, scalar_tail)                       \
    char* const begin = output;                                              \
    while (length >= 100U) {                                                 \
        __m256i v0 = encode_unpack(encode_load(input));                       \
        __m256i v1 = encode_unpack(encode_load(input + 24));                  \
        __m256i v2 = encode_unpack(encode_load(input + 48));                  \
        __m256i v3 = encode_unpack(encode_load(input + 72));                  \
        v0 = encode_translate(v0, url_safe);                                  \
        v1 = encode_translate(v1, url_safe);                                  \
        v2 = encode_translate(v2, url_safe);                                  \
        v3 = encode_translate(v3, url_safe);                                  \
        _mm256_storeu_si256((__m256i*)output, v0);                            \
        _mm256_storeu_si256((__m256i*)(output + 32), v1);                     \
        _mm256_storeu_si256((__m256i*)(output + 64), v2);                     \
        _mm256_storeu_si256((__m256i*)(output + 96), v3);                     \
        input += 96; output += 128; length -= 96;                             \
    }                                                                         \
    return (size_t)(output - begin) + scalar_tail(input, length, output)

size_t base64_avx2_encode(const unsigned char* input, size_t length,
                          char* output)
{
    BASE64_AVX2_ENCODE_BODY(0, base64_scalar_encode);
}

size_t base64url_avx2_encode(const unsigned char* input, size_t length,
                             char* output)
{
    BASE64_AVX2_ENCODE_BODY(1, base64url_scalar_encode);
}

#undef BASE64_AVX2_ENCODE_BODY

static BASE64_AVX2_INLINE __m256i decode_map_and_validate(__m256i input,
                                                     __m256i* invalid,
                                                     int checked)
{
    const __m256i association = _mm256_setr_epi8(
        1,1,1,1,1,1,1,1,0,0,0,0,0,12,0,18,
        1,1,1,1,1,1,1,1,0,0,0,0,0,12,0,18);
    const __m256i deltas = _mm256_setr_epi8(
        -71,0,0,19,4,-65,-65,-71,-71,17,0,-65,16,-71,-65,-32,
        -71,0,0,19,4,-65,-65,-71,-71,17,0,-65,16,-71,-65,-32);
    /* Validation tables adapted from simdutf's hybrid Base64 decoder.
     * See THIRD_PARTY_NOTICES.md. */
    const __m256i check_association = _mm256_setr_epi8(
        13,1,1,1,1,1,1,1,1,1,3,7,11,14,11,6,
        13,1,1,1,1,1,1,1,1,1,3,7,11,14,11,6);
    const __m256i check_values = _mm256_setr_epi8(
        -128,-128,-128,-128,-49,-65,-43,-90,
        -75,-95,0,-128,0,-128,0,-128,
        -128,-128,-128,-128,-49,-65,-43,-90,
        -75,-95,0,-128,0,-128,0,-128);
    const __m256i shifted = _mm256_srli_epi32(input, 3);
    const __m256i hash = _mm256_and_si256(
        _mm256_avg_epu8(_mm256_shuffle_epi8(association, input), shifted),
        _mm256_set1_epi8(0x0f));
    if (checked) {
        const __m256i check_hash = _mm256_avg_epu8(
            _mm256_shuffle_epi8(check_association, input), shifted);
        *invalid = _mm256_or_si256(*invalid, _mm256_adds_epi8(
            _mm256_shuffle_epi8(check_values, check_hash), input));
    }
    return _mm256_add_epi8(input, _mm256_shuffle_epi8(deltas, hash));
}

static BASE64_AVX2_INLINE __m256i decode_pack(__m256i value)
{
    value = _mm256_maddubs_epi16(value, _mm256_set1_epi32(0x01400140));
    value = _mm256_madd_epi16(value, _mm256_set1_epi32(0x00011000));
    return _mm256_shuffle_epi8(value, _mm256_setr_epi8(
        2,1,0,6,5,4,10,9,8,14,13,12,-1,-1,-1,-1,
        2,1,0,6,5,4,10,9,8,14,13,12,-1,-1,-1,-1));
}

static BASE64_AVX2_INLINE void decode_block(const unsigned char* input,
                                       unsigned char* output,
                                       __m256i* invalid, int checked)
{
    const __m256i source = _mm256_loadu_si256((const __m256i*)input);
    const __m256i mapped = decode_map_and_validate(source, invalid, checked);
    const __m256i value = decode_pack(mapped);
    _mm_storeu_si128((__m128i*)output, _mm256_castsi256_si128(value));
    _mm_storeu_si128((__m128i*)(output + 12),
                     _mm256_extracti128_si256(value, 1));
}

static BASE64_AVX2_INLINE size_t decode_avx2(const unsigned char* input,
                                        size_t length,
                                        unsigned char* output, int checked)
{
    unsigned char* const begin = output;
    __m256i invalid = _mm256_setzero_si256();
    while (length >= 136U) {
        decode_block(input, output, &invalid, checked);
        decode_block(input + 32, output + 24, &invalid, checked);
        decode_block(input + 64, output + 48, &invalid, checked);
        decode_block(input + 96, output + 72, &invalid, checked);
        input += 128; output += 96; length -= 128;
    }
    while (length >= 104U) {
        decode_block(input, output, &invalid, checked);
        decode_block(input + 32, output + 24, &invalid, checked);
        decode_block(input + 64, output + 48, &invalid, checked);
        input += 96; output += 72; length -= 96;
    }
    while (length >= 40U) {
        decode_block(input, output, &invalid, checked);
        input += 32; output += 24; length -= 32;
    }
    if (checked && _mm256_movemask_epi8(invalid) != 0)
        return BASE64_ERROR;
    {
        const size_t tail = checked
            ? base64_scalar_decode(input, length, output)
            : base64_scalar_decode_unchecked(input, length, output);
        return tail == BASE64_ERROR ? BASE64_ERROR
                                    : (size_t)(output - begin) + tail;
    }
}

size_t base64_avx2_decode(const unsigned char* input, size_t length,
                          unsigned char* output)
{
    return decode_avx2(input, length, output, 1);
}

size_t base64_avx2_decode_unchecked(const unsigned char* input, size_t length,
                                    unsigned char* output)
{
    return decode_avx2(input, length, output, 0);
}

#undef BASE64_AVX2_INLINE
