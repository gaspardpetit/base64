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
size_t base64_scalar_decode(const unsigned char*, size_t, unsigned char*, int);
size_t base64_scalar_decode_unchecked(const unsigned char*, size_t,
                                      unsigned char*, int);

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

/* After the first block, four preceding bytes are available. A single
 * overlapping load places 12 useful bytes in each 128-bit lane. The caller
 * must retain four bytes after the 24-byte block as well. */
static BASE64_AVX2_INLINE __m256i encode_load_overlap(const unsigned char* input)
{
    const __m256i value = _mm256_loadu_si256((const __m256i*)(input - 4));
    return _mm256_shuffle_epi8(value, _mm256_setr_epi8(
        5,4,6,5,8,7,9,8,11,10,12,11,14,13,15,14,
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

static BASE64_AVX2_INLINE __m128i encode_unpack_128(__m128i value)
{
    const __m128i a = _mm_mulhi_epu16(
        _mm_and_si128(value, _mm_set1_epi32(0x0fc0fc00)),
        _mm_set1_epi32(0x04000040));
    const __m128i b = _mm_mullo_epi16(
        _mm_and_si128(value, _mm_set1_epi32(0x003f03f0)),
        _mm_set1_epi32(0x01000010));
    return _mm_or_si128(a, b);
}

static BASE64_AVX2_INLINE __m128i encode_translate_128(
    __m128i value, int url_safe)
{
    __m128i index = _mm_subs_epu8(value, _mm_set1_epi8(51));
    index = _mm_sub_epi8(index, _mm_cmpgt_epi8(value, _mm_set1_epi8(25)));
    if (url_safe) {
        const __m128i offsets = _mm_setr_epi8(
            65,71,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-17,32,0,0);
        return _mm_add_epi8(value, _mm_shuffle_epi8(offsets, index));
    }
    else {
        const __m128i offsets = _mm_setr_epi8(
            65,71,-4,-4,-4,-4,-4,-4,-4,-4,-4,-4,-19,-16,0,0);
        return _mm_add_epi8(value, _mm_shuffle_epi8(offsets, index));
    }
}

static BASE64_AVX2_INLINE void encode_block_128(
    const unsigned char* input, char* output, int url_safe)
{
    __m128i value = _mm_loadu_si128((const __m128i*)input);
    value = _mm_shuffle_epi8(value, _mm_setr_epi8(
        1,0,2,1,4,3,5,4,7,6,8,7,10,9,11,10));
    value = encode_unpack_128(value);
    value = encode_translate_128(value, url_safe);
    _mm_storeu_si128((__m128i*)output, value);
}

static BASE64_AVX2_INLINE void encode_block_128_safe12(
    const unsigned char* input, char* output, int url_safe)
{
    uint32_t tail;
    __m128i value = _mm_loadl_epi64((const __m128i*)input);
    memcpy(&tail, input + 8, sizeof(tail));
    value = _mm_insert_epi32(value, (int)tail, 2);
    value = _mm_shuffle_epi8(value, _mm_setr_epi8(
        1,0,2,1,4,3,5,4,7,6,8,7,10,9,11,10));
    value = encode_unpack_128(value);
    value = encode_translate_128(value, url_safe);
    _mm_storeu_si128((__m128i*)output, value);
}

#define BASE64_AVX2_ENCODE_BODY(url_safe, scalar_tail)                       \
    char* const begin = output;                                              \
    if (length >= 100U) {                                                  \
        const __m256i first = encode_translate(                             \
            encode_unpack(encode_load(input)), url_safe);                   \
        _mm256_storeu_si256((__m256i*)output, first);                        \
        input += 24; output += 32; length -= 24;                            \
    }                                                                      \
    while (length >= 100U) {                                                 \
        __m256i v0 = encode_unpack(encode_load_overlap(input));                       \
        __m256i v1 = encode_unpack(encode_load_overlap(input + 24));                  \
        __m256i v2 = encode_unpack(encode_load_overlap(input + 48));                  \
        __m256i v3 = encode_unpack(encode_load_overlap(input + 72));                  \
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
    while (length >= 28U) {                                                   \
        __m256i value = encode_unpack(encode_load(input));                    \
        value = encode_translate(value, url_safe);                            \
        _mm256_storeu_si256((__m256i*)output, value);                         \
        input += 24; output += 32; length -= 24;                              \
    }                                                                         \
    if (length >= 16U) {                                                      \
        encode_block_128(input, output, url_safe);                            \
        input += 12; output += 16; length -= 12;                              \
    }                                                                         \
    if (length >= 12U) {                                                      \
        encode_block_128_safe12(input, output, url_safe);                     \
        input += 12; output += 16; length -= 12;                              \
    }                                                                         \
    return (size_t)(output - begin) +                                      \
        (url_safe                                                        \
            ? scalar_tail(input, length, output)                           \
            : base64_encode_inline_short(input, length, output))

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

size_t base64_avx128_encode(const unsigned char* input, size_t length,
                            char* output)
{
    encode_block_128(input, output, 0);
    return 16U + base64_encode_inline_short(
        input + 12U, length - 12U, output + 16U);
}

size_t base64url_avx128_encode(const unsigned char* input, size_t length,
                               char* output)
{
    encode_block_128(input, output, 1);
    return 16U + base64url_scalar_encode(
        input + 12U, length - 12U, output + 16U);
}

#undef BASE64_AVX2_ENCODE_BODY

static BASE64_AVX2_INLINE __m256i decode_map_and_validate(__m256i input,
                                                          __m256i* invalid,
                                                          int checked,
                                                          int support_url_safe)
{
    const __m256i shifted = _mm256_srli_epi32(input, 3);
    if (support_url_safe) {
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
    else {
        /* Standard-alphabet mapping and validation tables adapted from
         * simdutf. See THIRD_PARTY_NOTICES.md. */
        const __m256i association = _mm256_setr_epi8(
            1,1,1,1,1,1,1,1,0,0,0,0,0,15,0,15,
            1,1,1,1,1,1,1,1,0,0,0,0,0,15,0,15);
        const __m256i deltas = _mm256_setr_epi8(
            0,0,0,19,4,-65,-65,-71,-71,0,16,-61,-65,-65,-71,-71,
            0,0,0,19,4,-65,-65,-71,-71,0,16,-61,-65,-65,-71,-71);
        const __m256i check_association = _mm256_setr_epi8(
            13,1,1,1,1,1,1,1,1,1,3,7,11,11,11,15,
            13,1,1,1,1,1,1,1,1,1,3,7,11,11,11,15);
        const __m256i check_values = _mm256_setr_epi8(
            -128,-128,-128,-128,-49,-65,-43,-90,
            -75,-122,-47,-128,-79,-128,-111,-128,
            -128,-128,-128,-128,-49,-65,-43,-90,
            -75,-122,-47,-128,-79,-128,-111,-128);
        const __m256i hash = _mm256_avg_epu8(
            _mm256_shuffle_epi8(association, input), shifted);
        if (checked) {
            const __m256i check_hash = _mm256_avg_epu8(
                _mm256_shuffle_epi8(check_association, input), shifted);
            *invalid = _mm256_or_si256(*invalid, _mm256_adds_epi8(
                _mm256_shuffle_epi8(check_values, check_hash), input));
        }
        return _mm256_add_epi8(input, _mm256_shuffle_epi8(deltas, hash));
    }
}

static BASE64_AVX2_INLINE __m256i decode_pack(__m256i value)
{
    value = _mm256_maddubs_epi16(value, _mm256_set1_epi32(0x01400140));
    value = _mm256_madd_epi16(value, _mm256_set1_epi32(0x00011000));
    return _mm256_shuffle_epi8(value, _mm256_setr_epi8(
        2,1,0,6,5,4,10,9,8,14,13,12,-1,-1,-1,-1,
        2,1,0,6,5,4,10,9,8,14,13,12,-1,-1,-1,-1));
}

static BASE64_AVX2_INLINE __m128i decode_map_and_validate_128(
    __m128i input, __m128i* invalid, int checked, int support_url_safe)
{
    const __m128i shifted = _mm_srli_epi32(input, 3);
    const __m128i association = support_url_safe
        ? _mm_setr_epi8(1,1,1,1,1,1,1,1,0,0,0,0,0,12,0,18)
        : _mm_setr_epi8(1,1,1,1,1,1,1,1,0,0,0,0,0,15,0,15);
    const __m128i deltas = support_url_safe
        ? _mm_setr_epi8(-71,0,0,19,4,-65,-65,-71,
                        -71,17,0,-65,16,-71,-65,-32)
        : _mm_setr_epi8(0,0,0,19,4,-65,-65,-71,
                        -71,0,16,-61,-65,-65,-71,-71);
    const __m128i check_association = support_url_safe
        ? _mm_setr_epi8(13,1,1,1,1,1,1,1,1,1,3,7,11,14,11,6)
        : _mm_setr_epi8(13,1,1,1,1,1,1,1,1,1,3,7,11,11,11,15);
    const __m128i check_values = support_url_safe
        ? _mm_setr_epi8(-128,-128,-128,-128,-49,-65,-43,-90,
                        -75,-95,0,-128,0,-128,0,-128)
        : _mm_setr_epi8(-128,-128,-128,-128,-49,-65,-43,-90,
                        -75,-122,-47,-128,-79,-128,-111,-128);
    __m128i hash = _mm_avg_epu8(_mm_shuffle_epi8(association, input), shifted);
    if (support_url_safe)
        hash = _mm_and_si128(hash, _mm_set1_epi8(0x0f));
    if (checked) {
        const __m128i check_hash = _mm_avg_epu8(
            _mm_shuffle_epi8(check_association, input), shifted);
        *invalid = _mm_or_si128(*invalid, _mm_adds_epi8(
            _mm_shuffle_epi8(check_values, check_hash), input));
    }
    return _mm_add_epi8(input, _mm_shuffle_epi8(deltas, hash));
}

static BASE64_AVX2_INLINE __m128i decode_pack_128(__m128i value)
{
    value = _mm_maddubs_epi16(value, _mm_set1_epi32(0x01400140));
    value = _mm_madd_epi16(value, _mm_set1_epi32(0x00011000));
    return _mm_shuffle_epi8(value, _mm_setr_epi8(
        2,1,0,6,5,4,10,9,8,14,13,12,-1,-1,-1,-1));
}

static BASE64_AVX2_INLINE void decode_block_128(
    const unsigned char* input, unsigned char* output, __m128i* invalid,
    int checked, int support_url_safe)
{
    const __m128i source = _mm_loadu_si128((const __m128i*)input);
    const __m128i mapped = decode_map_and_validate_128(
        source, invalid, checked, support_url_safe);
    const __m128i value = decode_pack_128(mapped);
    _mm_storeu_si128((__m128i*)output, value);
}

static BASE64_AVX2_INLINE uint32_t contains_url_character4(
    const unsigned char* input)
{
    uint32_t word;
    uint32_t minus;
    uint32_t underscore;
    memcpy(&word, input, sizeof(word));
    minus = word ^ 0x2d2d2d2dU;
    underscore = word ^ 0x5f5f5f5fU;
    return ((minus - 0x01010101U) & ~minus & 0x80808080U) |
           ((underscore - 0x01010101U) & ~underscore & 0x80808080U);
}

static BASE64_AVX2_INLINE size_t decode_short_standard_tail(
    const unsigned char* input, size_t length, unsigned char* output,
    int checked)
{
    const unsigned char* p = input;
    unsigned char* const begin = output;
    size_t data_length = length;
    uint32_t invalid = 0;
    size_t quartets;
    size_t remainder;
    if (data_length && input[data_length - 1] == '=') {
        --data_length;
        if (data_length && input[data_length - 1] == '=')
            --data_length;
        if ((length & 3U) != 0U)
            return BASE64_ERROR;
    }
    if ((data_length & 3U) == 1U)
        return BASE64_ERROR;
    quartets = data_length >> 2;
    remainder = data_length & 3U;
#define DECODE_TAIL_QUARTET() do {                                         \
        const uint32_t value = base64_decode_0[p[0]] |                     \
            base64_decode_1[p[1]] | base64_decode_2[p[2]] |                \
            base64_decode_3[p[3]];                                         \
        invalid |= value;                                                   \
        invalid |= contains_url_character4(p) ? 0x01FFFFFFU : 0U;          \
        output[0] = (unsigned char)value;                                  \
        output[1] = (unsigned char)(value >> 8);                           \
        output[2] = (unsigned char)(value >> 16);                          \
        p += 4; output += 3;                                                \
    } while (0)
    switch (quartets) {
    case 5: DECODE_TAIL_QUARTET(); /* fall through */
    case 4: DECODE_TAIL_QUARTET(); /* fall through */
    case 3: DECODE_TAIL_QUARTET(); /* fall through */
    case 2: DECODE_TAIL_QUARTET(); /* fall through */
    case 1: DECODE_TAIL_QUARTET(); /* fall through */
    default: break;
    }
#undef DECODE_TAIL_QUARTET
    if (remainder != 0U) {
        uint32_t value = base64_decode_0[p[0]] | base64_decode_1[p[1]];
        if (remainder == 3U)
            value |= base64_decode_2[p[2]];
        invalid |= value;
        invalid |= (uint32_t)(p[0] == '-' || p[0] == '_' ||
            p[1] == '-' || p[1] == '_' ||
            (remainder == 3U && (p[2] == '-' || p[2] == '_'))) *
            0x01FFFFFFU;
        output[0] = (unsigned char)value;
        if (remainder == 3U)
            output[1] = (unsigned char)(value >> 8);
        output += remainder - 1U;
    }
    if ((checked && invalid >= 0x01FFFFFFU) ||
        (!checked && invalid == 0x01FFFFFFU))
        return BASE64_ERROR;
    return (size_t)(output - begin);
}

static BASE64_AVX2_INLINE size_t decode_short_anchored_standard(
    const unsigned char* input, size_t length, unsigned char* output,
    int checked)
{
    if (length == 36U) {
        __m128i invalid36 = _mm_setzero_si128();
        const __m128i first36 = decode_pack_128(decode_map_and_validate_128(
            _mm_loadu_si128((const __m128i*)input), &invalid36, checked, 0));
        const __m128i second36 = decode_pack_128(decode_map_and_validate_128(
            _mm_loadu_si128((const __m128i*)(input + 16)), &invalid36,
            checked, 0));
        size_t tail36;
        uint32_t high36;
        _mm_storeu_si128((__m128i*)output, first36);
        _mm_storel_epi64((__m128i*)(output + 12), second36);
        high36 = (uint32_t)_mm_extract_epi32(second36, 2);
        memcpy(output + 20, &high36, 4);
        if (checked && _mm_movemask_epi8(invalid36) != 0)
            return BASE64_ERROR;
        tail36 = decode_short_standard_tail(input + 32, 4, output + 24,
                                             checked);
        return tail36 == BASE64_ERROR ? BASE64_ERROR : 24U + tail36;
    }
    const size_t padding = input[length - 1] == '='
        ? 1U + (input[length - 2] == '=') : 0U;
    const size_t output_offset = (length - 16U) / 4U * 3U;
    __m128i invalid = _mm_setzero_si128();
    const __m128i first_source = _mm_loadu_si128((const __m128i*)input);
    const __m128i first = decode_pack_128(decode_map_and_validate_128(
        first_source, &invalid, checked, 0));
    __m128i last_source = _mm_loadu_si128(
        (const __m128i*)(input + length - 16U));
    const __m128i equals = _mm_cmpeq_epi8(last_source, _mm_set1_epi8('='));
    const unsigned equals_mask = (unsigned)_mm_movemask_epi8(equals);
    const unsigned expected_mask = padding == 2U ? 0xc000U
        : padding == 1U ? 0x8000U : 0U;
    last_source = _mm_or_si128(_mm_and_si128(equals, _mm_set1_epi8('A')),
                               _mm_andnot_si128(equals, last_source));
    {
        const __m128i last = decode_pack_128(decode_map_and_validate_128(
            last_source, &invalid, checked, 0));
        _mm_storeu_si128((__m128i*)output, first);
        _mm_storel_epi64((__m128i*)(output + output_offset), last);
        if (padding == 0U) {
            const uint32_t high = (uint32_t)_mm_extract_epi32(last, 2);
            memcpy(output + output_offset + 8U, &high, 4U);
        }
        else {
            const uint32_t high = (uint32_t)_mm_extract_epi32(last, 2);
            uint16_t pair = (uint16_t)high;
            memcpy(output + output_offset + 8U, &pair, 2U);
            if (padding == 1U)
                output[output_offset + 10U] = (unsigned char)(high >> 16);
        }
    }
    if (equals_mask != expected_mask ||
        (checked && _mm_movemask_epi8(invalid) != 0))
        return BASE64_ERROR;
    return length / 4U * 3U - padding;
}

static BASE64_AVX2_INLINE void decode_block(
    const unsigned char* input, unsigned char* output, __m256i* invalid,
    int checked, int support_url_safe);

static BASE64_AVX2_INLINE size_t decode_short_anchored_avx_standard(
    const unsigned char* input, size_t length, unsigned char* output,
    int checked)
{
    const size_t padding = input[length - 1] == '='
        ? 1U + (input[length - 2] == '=') : 0U;
    const size_t output_offset = (length - 16U) / 4U * 3U;
    __m256i invalid256 = _mm256_setzero_si256();
    __m128i invalid128 = _mm_setzero_si128();
    __m128i last_source;
    __m128i equals;
    __m128i last;
    unsigned equals_mask;
    unsigned expected_mask;
    uint32_t high;
    decode_block(input, output, &invalid256, checked, 0);
    last_source = _mm_loadu_si128(
        (const __m128i*)(input + length - 16U));
    equals = _mm_cmpeq_epi8(last_source, _mm_set1_epi8('='));
    equals_mask = (unsigned)_mm_movemask_epi8(equals);
    expected_mask = padding == 2U ? 0xc000U
        : padding == 1U ? 0x8000U : 0U;
    last_source = _mm_or_si128(_mm_and_si128(equals, _mm_set1_epi8('A')),
                               _mm_andnot_si128(equals, last_source));
    last = decode_pack_128(decode_map_and_validate_128(
        last_source, &invalid128, checked, 0));
    _mm_storel_epi64((__m128i*)(output + output_offset), last);
    high = (uint32_t)_mm_extract_epi32(last, 2);
    if (padding == 0U)
        memcpy(output + output_offset + 8U, &high, 4U);
    else {
        const uint16_t pair = (uint16_t)high;
        memcpy(output + output_offset + 8U, &pair, 2U);
        if (padding == 1U)
            output[output_offset + 10U] = (unsigned char)(high >> 16);
    }
    if (equals_mask != expected_mask ||
        (checked && (_mm256_movemask_epi8(invalid256) != 0 ||
                     _mm_movemask_epi8(invalid128) != 0)))
        return BASE64_ERROR;
    return length / 4U * 3U - padding;
}

static BASE64_AVX2_INLINE size_t decode_sse_anchored_standard(
    const unsigned char* input, size_t length, unsigned char* output,
    int checked, __m256i invalid256)
{
    const size_t padding = input[length - 1] == '='
        ? 1U + (input[length - 2] == '=') : 0U;
    const size_t last_offset = length - 16U;
    const size_t last_output_offset = last_offset / 4U * 3U;
    size_t offset = 0U;
    __m128i invalid = _mm_setzero_si128();
    __m128i last_source;
    __m128i equals;
    __m128i last;
    unsigned equals_mask;
    unsigned expected_mask;
    uint32_t high;
    decode_block(input, output, &invalid256, checked, 0);
    offset = 32U;
    if (length >= 80U) {
        decode_block(input + 32U, output + 24U, &invalid256, checked, 0);
        offset = 64U;
    }
    else if (length >= 64U) {
        decode_block_128(input + 32U, output + 24U, &invalid, checked, 0);
        offset = 48U;
    }
    last_source = _mm_loadu_si128((const __m128i*)(input + last_offset));
    equals = _mm_cmpeq_epi8(last_source, _mm_set1_epi8('='));
    equals_mask = (unsigned)_mm_movemask_epi8(equals);
    expected_mask = padding == 2U ? 0xc000U
        : padding == 1U ? 0x8000U : 0U;
    last_source = _mm_or_si128(_mm_and_si128(equals, _mm_set1_epi8('A')),
                               _mm_andnot_si128(equals, last_source));
    if (offset != last_offset) {
        const __m128i bridge = _mm_loadu_si128(
            (const __m128i*)(input + offset));
        __m256i pair = _mm256_inserti128_si256(
            _mm256_castsi128_si256(bridge), last_source, 1);
        uint32_t bridge_high;
        pair = decode_pack(decode_map_and_validate(
            pair, &invalid256, checked, 0));
        _mm_storel_epi64((__m128i*)(output + offset / 4U * 3U),
                         _mm256_castsi256_si128(pair));
        bridge_high = (uint32_t)_mm256_extract_epi32(pair, 2);
        memcpy(output + offset / 4U * 3U + 8U, &bridge_high, 4U);
        last = _mm256_extracti128_si256(pair, 1);
    }
    else {
        last = decode_pack_128(decode_map_and_validate_128(
            last_source, &invalid, checked, 0));
    }
    _mm_storel_epi64((__m128i*)(output + last_output_offset), last);
    high = (uint32_t)_mm_extract_epi32(last, 2);
    if (padding == 0U)
        memcpy(output + last_output_offset + 8U, &high, 4U);
    else {
        const uint16_t pair = (uint16_t)high;
        memcpy(output + last_output_offset + 8U, &pair, 2U);
        if (padding == 1U)
            output[last_output_offset + 10U] = (unsigned char)(high >> 16);
    }
    if (checked) {
        invalid256 = _mm256_or_si256(invalid256,
            _mm256_inserti128_si256(_mm256_setzero_si256(), invalid, 0));
    }
    if (equals_mask != expected_mask ||
        (checked && _mm256_movemask_epi8(invalid256) != 0))
        return BASE64_ERROR;
    return length / 4U * 3U - padding;
}

static BASE64_AVX2_INLINE size_t decode_projected_standard(
    const unsigned char* input, size_t length, unsigned char* output,
    int checked)
{
    unsigned char* const begin = output;
    __m256i invalid = _mm256_setzero_si256();
    while (length > 212U) {
        decode_block(input, output, &invalid, checked, 0);
        decode_block(input + 32U, output + 24U, &invalid, checked, 0);
        decode_block(input + 64U, output + 48U, &invalid, checked, 0);
        decode_block(input + 96U, output + 72U, &invalid, checked, 0);
        input += 128U;
        output += 96U;
        length -= 128U;
    }
    while (length > 84U) {
        decode_block(input, output, &invalid, checked, 0);
        input += 32U;
        output += 24U;
        length -= 32U;
    }
    {
        const size_t tail = decode_sse_anchored_standard(
            input, length, output, checked, invalid);
        return tail == BASE64_ERROR ? BASE64_ERROR
                                    : (size_t)(output - begin) + tail;
    }
}

static BASE64_AVX2_INLINE void decode_block(const unsigned char* input,
                                             unsigned char* output,
                                             __m256i* invalid, int checked,
                                             int support_url_safe)
{
    const __m256i source = _mm256_loadu_si256((const __m256i*)input);
    const __m256i mapped = decode_map_and_validate(
        source, invalid, checked, support_url_safe);
    const __m256i value = decode_pack(mapped);
    _mm_storeu_si128((__m128i*)output, _mm256_castsi256_si128(value));
    _mm_storeu_si128((__m128i*)(output + 12),
                     _mm256_extracti128_si256(value, 1));
}

static BASE64_AVX2_INLINE size_t decode_avx2(const unsigned char* input,
                                              size_t length,
                                              unsigned char* output,
                                              int checked,
                                              int support_url_safe)
{
    unsigned char* const begin = output;
    if (!support_url_safe && length >= 24U && length <= 36U &&
        (length & 3U) == 0U)
        return decode_short_anchored_standard(input, length, output, checked);
    if (!support_url_safe && (length == 40U || length == 44U))
        return decode_short_anchored_avx_standard(input, length, output,
                                                  checked);
    if (!support_url_safe && length >= 48U && length <= 84U &&
        (length & 3U) == 0U)
        return decode_sse_anchored_standard(
            input, length, output, checked, _mm256_setzero_si256());
    if (!support_url_safe && length > 84U && (length & 3U) == 0U)
        return decode_projected_standard(input, length, output, checked);
    __m256i invalid = _mm256_setzero_si256();
    while (length >= 136U) {
        decode_block(input, output, &invalid, checked, support_url_safe);
        decode_block(input + 32, output + 24, &invalid, checked,
                     support_url_safe);
        decode_block(input + 64, output + 48, &invalid, checked,
                     support_url_safe);
        decode_block(input + 96, output + 72, &invalid, checked,
                     support_url_safe);
        input += 128; output += 96; length -= 128;
    }
    while (length >= 104U) {
        decode_block(input, output, &invalid, checked, support_url_safe);
        decode_block(input + 32, output + 24, &invalid, checked,
                     support_url_safe);
        decode_block(input + 64, output + 48, &invalid, checked,
                     support_url_safe);
        input += 96; output += 72; length -= 96;
    }
    while (length >= 40U) {
        decode_block(input, output, &invalid, checked, support_url_safe);
        input += 32; output += 24; length -= 32;
    }
    if (length >= 24U) {
        __m128i invalid128 = _mm256_castsi256_si128(invalid);
        do {
            decode_block_128(input, output, &invalid128, checked,
                             support_url_safe);
            input += 16; output += 12; length -= 16;
        } while (length >= 24U);
        invalid = _mm256_inserti128_si256(invalid, invalid128, 0);
    }
    if (checked && _mm256_movemask_epi8(invalid) != 0)
        return BASE64_ERROR;
    {
        const size_t tail = !support_url_safe && length <= 20U
            ? decode_short_standard_tail(input, length, output, checked)
            : (checked
                ? base64_scalar_decode(input, length, output,
                                       support_url_safe)
                : base64_scalar_decode_unchecked(input, length, output,
                                                 support_url_safe));
        return tail == BASE64_ERROR ? BASE64_ERROR
                                    : (size_t)(output - begin) + tail;
    }
}

size_t base64_avx2_decode(const unsigned char* input, size_t length,
                          unsigned char* output)
{
    return decode_avx2(input, length, output, 1, 1);
}

size_t base64_avx2_decode_unchecked(const unsigned char* input, size_t length,
                                    unsigned char* output)
{
    return decode_avx2(input, length, output, 0, 1);
}

size_t base64_avx2_decode_standard(const unsigned char* input, size_t length,
                                   unsigned char* output)
{
    return decode_avx2(input, length, output, 1, 0);
}

size_t base64_avx2_decode_standard_unchecked(const unsigned char* input,
                                             size_t length,
                                             unsigned char* output)
{
    return decode_avx2(input, length, output, 0, 0);
}

#undef BASE64_AVX2_INLINE
