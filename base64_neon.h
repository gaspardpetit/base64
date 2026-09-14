/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 Gaspard Petit
 */

#ifndef BASE64_NEON_H_INCLUDED
#define BASE64_NEON_H_INCLUDED

/* AArch64 requires Advanced SIMD (NEON).  Do not enable this backend merely
 * from an ARM architecture macro: the implementation uses AArch64 table
 * lookup instructions. */
#if defined(__aarch64__)

#include <stddef.h>
#include <stdint.h>
#include <arm_neon.h>

#if defined(BASE64_NEON_HEADER_ONLY)
#  define BASE64_NEON_API static inline
#elif defined(BASE64_NEON_IMPLEMENTATION)
#  define BASE64_NEON_API
#else
#  define BASE64_NEON_API extern
#endif

/* These process complete SIMD blocks only. The scalar frontend owns tails and
 * padding, so callers pass a multiple of 48 / 64 bytes respectively. */
BASE64_NEON_API size_t base64_neon_encode_blocks(
    const unsigned char* input, size_t length, char* output);
BASE64_NEON_API size_t base64url_neon_encode_blocks(
    const unsigned char* input, size_t length, char* output);
BASE64_NEON_API int base64_neon_decode_blocks(
    const unsigned char* input, size_t length, unsigned char* output);
BASE64_NEON_API void base64_neon_decode_blocks_unchecked(
    const unsigned char* input, size_t length, unsigned char* output);

#if defined(BASE64_NEON_IMPLEMENTATION) || defined(BASE64_NEON_HEADER_ONLY)

static inline uint8x16_t base64_neon_decode_map(uint8x16_t chars,
                                                  uint8x16_t* minimum,
                                                  uint8x16_t lo_table,
                                                  uint8x16_t hi_table,
                                                  uint8x16_t roll_table,
                                                  uint8x16_t low_mask)
{
    uint8x16_t classes = vandq_u8(
        vqtbl1q_u8(lo_table, vandq_u8(chars, low_mask)),
        vqtbl1q_u8(hi_table, vshrq_n_u8(chars, 3)));
    const uint8x16_t dash = vceqq_u8(chars, vdupq_n_u8('-'));
    const uint8x16_t underscore = vceqq_u8(chars, vdupq_n_u8('_'));
    classes = vbslq_u8(dash, vdupq_n_u8(2), classes);
    classes = vbslq_u8(underscore, vdupq_n_u8(4), classes);
    *minimum = vminq_u8(*minimum, classes);
    chars = vaddq_u8(chars, vqtbl1q_u8(roll_table, vclzq_u8(classes)));
    chars = vbslq_u8(dash, vdupq_n_u8(62), chars);
    return vbslq_u8(underscore, vdupq_n_u8(63), chars);
}

static inline size_t base64_neon_encode_blocks_impl(
    const unsigned char* input, size_t length, char* output,
    const uint8_t* alphabet)
{
    const uint8x16x4_t table = {{
        vld1q_u8(alphabet), vld1q_u8(alphabet + 16),
        vld1q_u8(alphabet + 32), vld1q_u8(alphabet + 48)}};
    const uint8x16_t six_bits = vdupq_n_u8(63);
    const unsigned char* const begin = input;

    while (length >= 48U) {
        const uint8x16x3_t bytes = vld3q_u8(input);
        const uint8x16_t a = bytes.val[0], b = bytes.val[1], c = bytes.val[2];
        const uint8x16_t i0 = vshrq_n_u8(a, 2);
        const uint8x16_t i1 = vandq_u8(vsliq_n_u8(vshrq_n_u8(b, 4), a, 4), six_bits);
        const uint8x16_t i2 = vandq_u8(vsliq_n_u8(vshrq_n_u8(c, 6), b, 2), six_bits);
        const uint8x16_t i3 = vandq_u8(c, six_bits);
        const uint8x16x4_t chars = {{
            vqtbl4q_u8(table, i0), vqtbl4q_u8(table, i1),
            vqtbl4q_u8(table, i2), vqtbl4q_u8(table, i3)}};
        vst4q_u8((uint8_t*)output, chars);
        input += 48;
        output += 64;
        length -= 48;
    }
    return (size_t)(input - begin);
}

BASE64_NEON_API size_t base64_neon_encode_blocks(
    const unsigned char* input, size_t length, char* output)
{
    static const uint8_t alphabet[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    return base64_neon_encode_blocks_impl(input, length, output, alphabet);
}

BASE64_NEON_API size_t base64url_neon_encode_blocks(
    const unsigned char* input, size_t length, char* output)
{
    static const uint8_t alphabet[65] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    return base64_neon_encode_blocks_impl(input, length, output, alphabet);
}

BASE64_NEON_API int base64_neon_decode_blocks(
    const unsigned char* input, size_t length, unsigned char* output)
{
    static const uint8_t lut_lo[16] = {
        0xa9, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8,
        0xf8, 0xf9, 0xf1, 0xa2, 0xa1, 0xa1, 0xa0, 0xa4};
    static const uint8_t lut_hi[16] = {
        0x00, 0x01, 0x00, 0x00, 0x01, 0x06, 0x08, 0x08,
        0x10, 0x20, 0x20, 0x10, 0x40, 0x80, 0x80, 0x40};
    static const uint8_t roll_lut[16] = {
        0xb9, 0xb9, 0xbf, 0xbf, 0x04, 0x10, 0x13, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const uint8x16_t lo_table = vld1q_u8(lut_lo);
    const uint8x16_t hi_table = vld1q_u8(lut_hi);
    const uint8x16_t roll_table = vld1q_u8(roll_lut);
    const uint8x16_t low_mask = vdupq_n_u8(15);
    uint8x16_t minimum = vdupq_n_u8(255);

    while (length >= 64U) {
        const uint8x16x4_t chars = vld4q_u8(input);
        const uint8x16_t a = base64_neon_decode_map(chars.val[0], &minimum, lo_table, hi_table, roll_table, low_mask);
        const uint8x16_t b = base64_neon_decode_map(chars.val[1], &minimum, lo_table, hi_table, roll_table, low_mask);
        const uint8x16_t c = base64_neon_decode_map(chars.val[2], &minimum, lo_table, hi_table, roll_table, low_mask);
        const uint8x16_t d = base64_neon_decode_map(chars.val[3], &minimum, lo_table, hi_table, roll_table, low_mask);
        const uint8x16x3_t bytes = {{
            vsliq_n_u8(vshrq_n_u8(b, 4), a, 2),
            vsliq_n_u8(vshrq_n_u8(c, 2), b, 4),
            vsliq_n_u8(d, c, 6)}};
        vst3q_u8(output, bytes);
        input += 64;
        output += 48;
        length -= 64;
    }
    /* Valid alphabet entries have a class value above one. This deliberately
     * rejects whitespace, matching base64_decode's existing strict contract. */
    return vminvq_u8(minimum) > 1;
}

BASE64_NEON_API void base64_neon_decode_blocks_unchecked(
    const unsigned char* input, size_t length, unsigned char* output)
{
    static const uint8_t offsets[16] = {
        0, 0, 0, 4, (uint8_t)-65, (uint8_t)-65, (uint8_t)-71, (uint8_t)-71,
        0, 0, 0, 0, 0, 0, 0, 0};
    const uint8x16_t offset_table = vld1q_u8(offsets);
    const uint8x16_t plus = vdupq_n_u8('+');
    const uint8x16_t slash = vdupq_n_u8('/');
    const uint8x16_t dash = vdupq_n_u8('-');
    const uint8x16_t underscore = vdupq_n_u8('_');
    const uint8x16_t value_62 = vdupq_n_u8(62);
    const uint8x16_t value_63 = vdupq_n_u8(63);

    while (length >= 64U) {
        const uint8x16x4_t chars = vld4q_u8(input);
        uint8x16_t a = vaddq_u8(chars.val[0], vqtbl1q_u8(offset_table, vshrq_n_u8(chars.val[0], 4)));
        uint8x16_t b = vaddq_u8(chars.val[1], vqtbl1q_u8(offset_table, vshrq_n_u8(chars.val[1], 4)));
        uint8x16_t c = vaddq_u8(chars.val[2], vqtbl1q_u8(offset_table, vshrq_n_u8(chars.val[2], 4)));
        uint8x16_t d = vaddq_u8(chars.val[3], vqtbl1q_u8(offset_table, vshrq_n_u8(chars.val[3], 4)));
        a = vbslq_u8(vceqq_u8(chars.val[0], plus), value_62, a);
        b = vbslq_u8(vceqq_u8(chars.val[1], plus), value_62, b);
        c = vbslq_u8(vceqq_u8(chars.val[2], plus), value_62, c);
        d = vbslq_u8(vceqq_u8(chars.val[3], plus), value_62, d);
        a = vbslq_u8(vceqq_u8(chars.val[0], slash), value_63, a);
        b = vbslq_u8(vceqq_u8(chars.val[1], slash), value_63, b);
        c = vbslq_u8(vceqq_u8(chars.val[2], slash), value_63, c);
        d = vbslq_u8(vceqq_u8(chars.val[3], slash), value_63, d);
        a = vbslq_u8(vceqq_u8(chars.val[0], dash), value_62, a);
        b = vbslq_u8(vceqq_u8(chars.val[1], dash), value_62, b);
        c = vbslq_u8(vceqq_u8(chars.val[2], dash), value_62, c);
        d = vbslq_u8(vceqq_u8(chars.val[3], dash), value_62, d);
        a = vbslq_u8(vceqq_u8(chars.val[0], underscore), value_63, a);
        b = vbslq_u8(vceqq_u8(chars.val[1], underscore), value_63, b);
        c = vbslq_u8(vceqq_u8(chars.val[2], underscore), value_63, c);
        d = vbslq_u8(vceqq_u8(chars.val[3], underscore), value_63, d);
        const uint8x16x3_t bytes = {{
            vsliq_n_u8(vshrq_n_u8(b, 4), a, 2),
            vsliq_n_u8(vshrq_n_u8(c, 2), b, 4),
            vsliq_n_u8(d, c, 6)}};
        vst3q_u8(output, bytes);
        input += 64;
        output += 48;
        length -= 64;
    }
}

#endif

#undef BASE64_NEON_API
#endif /* __aarch64__ */
#endif /* BASE64_NEON_H_INCLUDED */
