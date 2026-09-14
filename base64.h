/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 Gaspard Petit
 */

#ifndef BASE64_H_INCLUDED
#define BASE64_H_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define BASE64_ERROR ((size_t)-1)

#if defined(BASE64_IMPLEMENTATION)
#  define BASE64_API
#  define BASE64_INCLUDE_IMPLEMENTATION
#elif defined(BASE64_HEADER_ONLY)
#  define BASE64_API static inline
#  define BASE64_INCLUDE_IMPLEMENTATION
#else
#  define BASE64_API extern
#endif

#ifdef __cplusplus
extern "C" {
#endif

BASE64_API size_t base64_encoded_size(size_t input_size);
BASE64_API size_t base64_decoded_max_size(size_t input_size);
BASE64_API size_t base64_encode(const unsigned char* input, size_t length,
                                char* output);
BASE64_API size_t base64url_encode(const unsigned char* input, size_t length,
                                   char* output);
/* Set support_url_safe to zero for standard '+'/'/' Base64 only. On NEON this
 * selects the standard decoder without a URL-safe detection pass. */
#if !defined(BASE64_HEADER_ONLY)
extern const uint32_t base64_decode_0[256];
extern const uint32_t base64_decode_1[256];
extern const uint32_t base64_decode_2[256];
extern const uint32_t base64_decode_3[256];
#endif
#if !defined(BASE64_IMPLEMENTATION) && !defined(BASE64_HEADER_ONLY)
extern size_t base64_decode_compiled(const unsigned char*, size_t,
                                     unsigned char*, int);
extern size_t base64_decode_unchecked_compiled(const unsigned char*, size_t,
                                               unsigned char*, int);
#else
BASE64_API size_t base64_decode(const unsigned char* input, size_t length,
                                unsigned char* output, int support_url_safe);
BASE64_API size_t base64_decode_unchecked(const unsigned char* input,
                                          size_t length,
                                          unsigned char* output,
                                          int support_url_safe);
#endif

#ifdef __cplusplus
}
#endif

#ifdef BASE64_INCLUDE_IMPLEMENTATION

#if defined(_MSC_VER)
#  define BASE64_RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
#  define BASE64_RESTRICT __restrict__
#elif defined(__cplusplus)
#  define BASE64_RESTRICT
#else
#  define BASE64_RESTRICT restrict
#endif

static const char base64_alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char base64url_alphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

#define BASE64_PAIR_ROW(first)                                              \
    {                                                                       \
        {first, 'A'}, {first, 'B'}, {first, 'C'}, {first, 'D'},             \
        {first, 'E'}, {first, 'F'}, {first, 'G'}, {first, 'H'},             \
        {first, 'I'}, {first, 'J'}, {first, 'K'}, {first, 'L'},             \
        {first, 'M'}, {first, 'N'}, {first, 'O'}, {first, 'P'},             \
        {first, 'Q'}, {first, 'R'}, {first, 'S'}, {first, 'T'},             \
        {first, 'U'}, {first, 'V'}, {first, 'W'}, {first, 'X'},             \
        {first, 'Y'}, {first, 'Z'}, {first, 'a'}, {first, 'b'},             \
        {first, 'c'}, {first, 'd'}, {first, 'e'}, {first, 'f'},             \
        {first, 'g'}, {first, 'h'}, {first, 'i'}, {first, 'j'},             \
        {first, 'k'}, {first, 'l'}, {first, 'm'}, {first, 'n'},             \
        {first, 'o'}, {first, 'p'}, {first, 'q'}, {first, 'r'},             \
        {first, 's'}, {first, 't'}, {first, 'u'}, {first, 'v'},             \
        {first, 'w'}, {first, 'x'}, {first, 'y'}, {first, 'z'},             \
        {first, '0'}, {first, '1'}, {first, '2'}, {first, '3'},             \
        {first, '4'}, {first, '5'}, {first, '6'}, {first, '7'},             \
        {first, '8'}, {first, '9'}, {first, BASE64_CHAR_62},                \
        {first, BASE64_CHAR_63}                                             \
    }

#define BASE64_CHAR_62 '+'
#define BASE64_CHAR_63 '/'
static const char base64_encode_pair[64][64][2] = {
    BASE64_PAIR_ROW('A'), BASE64_PAIR_ROW('B'), BASE64_PAIR_ROW('C'),
    BASE64_PAIR_ROW('D'), BASE64_PAIR_ROW('E'), BASE64_PAIR_ROW('F'),
    BASE64_PAIR_ROW('G'), BASE64_PAIR_ROW('H'), BASE64_PAIR_ROW('I'),
    BASE64_PAIR_ROW('J'), BASE64_PAIR_ROW('K'), BASE64_PAIR_ROW('L'),
    BASE64_PAIR_ROW('M'), BASE64_PAIR_ROW('N'), BASE64_PAIR_ROW('O'),
    BASE64_PAIR_ROW('P'), BASE64_PAIR_ROW('Q'), BASE64_PAIR_ROW('R'),
    BASE64_PAIR_ROW('S'), BASE64_PAIR_ROW('T'), BASE64_PAIR_ROW('U'),
    BASE64_PAIR_ROW('V'), BASE64_PAIR_ROW('W'), BASE64_PAIR_ROW('X'),
    BASE64_PAIR_ROW('Y'), BASE64_PAIR_ROW('Z'), BASE64_PAIR_ROW('a'),
    BASE64_PAIR_ROW('b'), BASE64_PAIR_ROW('c'), BASE64_PAIR_ROW('d'),
    BASE64_PAIR_ROW('e'), BASE64_PAIR_ROW('f'), BASE64_PAIR_ROW('g'),
    BASE64_PAIR_ROW('h'), BASE64_PAIR_ROW('i'), BASE64_PAIR_ROW('j'),
    BASE64_PAIR_ROW('k'), BASE64_PAIR_ROW('l'), BASE64_PAIR_ROW('m'),
    BASE64_PAIR_ROW('n'), BASE64_PAIR_ROW('o'), BASE64_PAIR_ROW('p'),
    BASE64_PAIR_ROW('q'), BASE64_PAIR_ROW('r'), BASE64_PAIR_ROW('s'),
    BASE64_PAIR_ROW('t'), BASE64_PAIR_ROW('u'), BASE64_PAIR_ROW('v'),
    BASE64_PAIR_ROW('w'), BASE64_PAIR_ROW('x'), BASE64_PAIR_ROW('y'),
    BASE64_PAIR_ROW('z'), BASE64_PAIR_ROW('0'), BASE64_PAIR_ROW('1'),
    BASE64_PAIR_ROW('2'), BASE64_PAIR_ROW('3'), BASE64_PAIR_ROW('4'),
    BASE64_PAIR_ROW('5'), BASE64_PAIR_ROW('6'), BASE64_PAIR_ROW('7'),
    BASE64_PAIR_ROW('8'), BASE64_PAIR_ROW('9'), BASE64_PAIR_ROW('+'),
    BASE64_PAIR_ROW('/')
};

#undef BASE64_CHAR_62
#undef BASE64_CHAR_63
#define BASE64_CHAR_62 '-'
#define BASE64_CHAR_63 '_'
static const char base64url_encode_pair[64][64][2] = {
    BASE64_PAIR_ROW('A'), BASE64_PAIR_ROW('B'), BASE64_PAIR_ROW('C'),
    BASE64_PAIR_ROW('D'), BASE64_PAIR_ROW('E'), BASE64_PAIR_ROW('F'),
    BASE64_PAIR_ROW('G'), BASE64_PAIR_ROW('H'), BASE64_PAIR_ROW('I'),
    BASE64_PAIR_ROW('J'), BASE64_PAIR_ROW('K'), BASE64_PAIR_ROW('L'),
    BASE64_PAIR_ROW('M'), BASE64_PAIR_ROW('N'), BASE64_PAIR_ROW('O'),
    BASE64_PAIR_ROW('P'), BASE64_PAIR_ROW('Q'), BASE64_PAIR_ROW('R'),
    BASE64_PAIR_ROW('S'), BASE64_PAIR_ROW('T'), BASE64_PAIR_ROW('U'),
    BASE64_PAIR_ROW('V'), BASE64_PAIR_ROW('W'), BASE64_PAIR_ROW('X'),
    BASE64_PAIR_ROW('Y'), BASE64_PAIR_ROW('Z'), BASE64_PAIR_ROW('a'),
    BASE64_PAIR_ROW('b'), BASE64_PAIR_ROW('c'), BASE64_PAIR_ROW('d'),
    BASE64_PAIR_ROW('e'), BASE64_PAIR_ROW('f'), BASE64_PAIR_ROW('g'),
    BASE64_PAIR_ROW('h'), BASE64_PAIR_ROW('i'), BASE64_PAIR_ROW('j'),
    BASE64_PAIR_ROW('k'), BASE64_PAIR_ROW('l'), BASE64_PAIR_ROW('m'),
    BASE64_PAIR_ROW('n'), BASE64_PAIR_ROW('o'), BASE64_PAIR_ROW('p'),
    BASE64_PAIR_ROW('q'), BASE64_PAIR_ROW('r'), BASE64_PAIR_ROW('s'),
    BASE64_PAIR_ROW('t'), BASE64_PAIR_ROW('u'), BASE64_PAIR_ROW('v'),
    BASE64_PAIR_ROW('w'), BASE64_PAIR_ROW('x'), BASE64_PAIR_ROW('y'),
    BASE64_PAIR_ROW('z'), BASE64_PAIR_ROW('0'), BASE64_PAIR_ROW('1'),
    BASE64_PAIR_ROW('2'), BASE64_PAIR_ROW('3'), BASE64_PAIR_ROW('4'),
    BASE64_PAIR_ROW('5'), BASE64_PAIR_ROW('6'), BASE64_PAIR_ROW('7'),
    BASE64_PAIR_ROW('8'), BASE64_PAIR_ROW('9'), BASE64_PAIR_ROW('-'),
    BASE64_PAIR_ROW('_')
};

#undef BASE64_CHAR_62
#undef BASE64_CHAR_63
#undef BASE64_PAIR_ROW

#define BASE64_ENCODE_PAIR(index)                                           \
    (((const char (*)[2])base64_encode_pair)[index])
#define BASE64URL_ENCODE_PAIR(index)                                        \
    (((const char (*)[2])base64url_encode_pair)[index])

static inline void base64_encode_quantum(const unsigned char* input,
                                         char* output)
{
    const uint32_t value = ((uint32_t)input[0] << 16) |
                           ((uint32_t)input[1] << 8) | input[2];
    memcpy(output, BASE64_ENCODE_PAIR(value >> 12), 2);
    memcpy(output + 2, BASE64_ENCODE_PAIR(value & 0xFFFU), 2);
}

BASE64_API size_t base64_encoded_size(size_t input_size)
{
    return ((input_size + 2U) / 3U) * 4U;
}

BASE64_API size_t base64_decoded_max_size(size_t input_size)
{
    const size_t remainder = input_size % 4U;
    return (input_size / 4U) * 3U +
           (remainder != 0U ? remainder - 1U : 0U);
}

BASE64_API size_t base64_encode(const unsigned char* BASE64_RESTRICT input,
                                size_t length,
                                char* BASE64_RESTRICT output)
{
    char* const begin = output;

    while (length >= 12U) {
        const uint32_t v0 = ((uint32_t)input[0] << 16) |
                            ((uint32_t)input[1] << 8) | input[2];
        const uint32_t v1 = ((uint32_t)input[3] << 16) |
                            ((uint32_t)input[4] << 8) | input[5];
        const uint32_t v2 = ((uint32_t)input[6] << 16) |
                            ((uint32_t)input[7] << 8) | input[8];
        const uint32_t v3 = ((uint32_t)input[9] << 16) |
                            ((uint32_t)input[10] << 8) | input[11];
        memcpy(output, BASE64_ENCODE_PAIR(v0 >> 12), 2);
        memcpy(output + 2, BASE64_ENCODE_PAIR(v0 & 0xFFFU), 2);
        memcpy(output + 4, BASE64_ENCODE_PAIR(v1 >> 12), 2);
        memcpy(output + 6, BASE64_ENCODE_PAIR(v1 & 0xFFFU), 2);
        memcpy(output + 8, BASE64_ENCODE_PAIR(v2 >> 12), 2);
        memcpy(output + 10, BASE64_ENCODE_PAIR(v2 & 0xFFFU), 2);
        memcpy(output + 12, BASE64_ENCODE_PAIR(v3 >> 12), 2);
        memcpy(output + 14, BASE64_ENCODE_PAIR(v3 & 0xFFFU), 2);
        input += 12;
        output += 16;
        length -= 12;
    }
    while (length >= 3U) {
        base64_encode_quantum(input, output);
        input += 3;
        output += 4;
        length -= 3;
    }
    if (length != 0U) {
        output[0] = base64_alphabet[input[0] >> 2];
        output[1] = base64_alphabet[((input[0] & 3U) << 4) |
                                    (length == 2U ? input[1] >> 4 : 0U)];
        output[2] = length == 2U
                        ? base64_alphabet[(input[1] & 15U) << 2] : '=';
        output[3] = '=';
        output += 4;
    }
    return (size_t)(output - begin);
}

BASE64_API size_t base64url_encode(
    const unsigned char* BASE64_RESTRICT input,
    size_t length,
    char* BASE64_RESTRICT output)
{
    char* const begin = output;

    while (length >= 12U) {
        const uint32_t v0 = ((uint32_t)input[0] << 16) |
                            ((uint32_t)input[1] << 8) | input[2];
        const uint32_t v1 = ((uint32_t)input[3] << 16) |
                            ((uint32_t)input[4] << 8) | input[5];
        const uint32_t v2 = ((uint32_t)input[6] << 16) |
                            ((uint32_t)input[7] << 8) | input[8];
        const uint32_t v3 = ((uint32_t)input[9] << 16) |
                            ((uint32_t)input[10] << 8) | input[11];
        memcpy(output, BASE64URL_ENCODE_PAIR(v0 >> 12), 2);
        memcpy(output + 2, BASE64URL_ENCODE_PAIR(v0 & 0xFFFU), 2);
        memcpy(output + 4, BASE64URL_ENCODE_PAIR(v1 >> 12), 2);
        memcpy(output + 6, BASE64URL_ENCODE_PAIR(v1 & 0xFFFU), 2);
        memcpy(output + 8, BASE64URL_ENCODE_PAIR(v2 >> 12), 2);
        memcpy(output + 10, BASE64URL_ENCODE_PAIR(v2 & 0xFFFU), 2);
        memcpy(output + 12, BASE64URL_ENCODE_PAIR(v3 >> 12), 2);
        memcpy(output + 14, BASE64URL_ENCODE_PAIR(v3 & 0xFFFU), 2);
        input += 12;
        output += 16;
        length -= 12;
    }
    while (length >= 3U) {
        const uint32_t value = ((uint32_t)input[0] << 16) |
                               ((uint32_t)input[1] << 8) | input[2];
        memcpy(output, BASE64URL_ENCODE_PAIR(value >> 12), 2);
        memcpy(output + 2, BASE64URL_ENCODE_PAIR(value & 0xFFFU), 2);
        input += 3;
        output += 4;
        length -= 3;
    }
    if (length != 0U) {
        output[0] = base64url_alphabet[input[0] >> 2];
        output[1] = base64url_alphabet[((input[0] & 3U) << 4) |
                                       (length == 2U ? input[1] >> 4 : 0U)];
        output[2] = length == 2U
                        ? base64url_alphabet[(input[1] & 15U) << 2] : '=';
        output[3] = '=';
        output += 4;
    }
    return (size_t)(output - begin);
}

#define BASE64_BAD_CHARACTER 0x01FFFFFFU
#define BASE64_INVALID_VALUE 64U
#define BASE64_REPEAT_16(X, v)                                              \
    X(v), X(v), X(v), X(v), X(v), X(v), X(v), X(v),                       \
    X(v), X(v), X(v), X(v), X(v), X(v), X(v), X(v)
#define BASE64_REPEAT_8(X, v) X(v), X(v), X(v), X(v), X(v), X(v), X(v), X(v)
#define BASE64_DECODE_VALUES(X)                                             \
    BASE64_REPEAT_16(X, 64), BASE64_REPEAT_16(X, 64),                      \
    BASE64_REPEAT_8(X, 64), X(64), X(64), X(64), X(62), X(64), X(62),     \
    X(64), X(63), X(52), X(53), X(54), X(55), X(56), X(57), X(58),       \
    X(59), X(60), X(61), X(64), X(64), X(64), X(64), X(64), X(64),       \
    X(64), X(0), X(1), X(2), X(3), X(4), X(5), X(6), X(7), X(8),         \
    X(9), X(10), X(11), X(12), X(13), X(14), X(15), X(16), X(17),        \
    X(18), X(19), X(20), X(21), X(22), X(23), X(24), X(25),              \
    X(64), X(64), X(64), X(64), X(63), X(64), X(26), X(27), X(28),       \
    X(29), X(30), X(31), X(32), X(33), X(34), X(35), X(36), X(37),       \
    X(38), X(39), X(40), X(41), X(42), X(43), X(44), X(45), X(46),       \
    X(47), X(48), X(49), X(50), X(51),                                    \
    BASE64_REPEAT_16(X, 64), BASE64_REPEAT_16(X, 64),                      \
    BASE64_REPEAT_16(X, 64), BASE64_REPEAT_16(X, 64),                      \
    BASE64_REPEAT_16(X, 64), BASE64_REPEAT_16(X, 64),                      \
    BASE64_REPEAT_16(X, 64), BASE64_REPEAT_16(X, 64),                      \
    X(64), X(64), X(64), X(64), X(64)

#define BASE64_TABLE_0(v) ((v) == BASE64_INVALID_VALUE                     \
    ? BASE64_BAD_CHARACTER : (uint32_t)(v) << 2)
#if defined(BASE64_HEADER_ONLY)
#  define BASE64_DECODE_TABLE_STORAGE static const
#else
#  define BASE64_DECODE_TABLE_STORAGE extern const
#endif
BASE64_DECODE_TABLE_STORAGE uint32_t base64_decode_0[256] = {
    BASE64_DECODE_VALUES(BASE64_TABLE_0)
};
#undef BASE64_TABLE_0

#define BASE64_TABLE_1(v) ((v) == BASE64_INVALID_VALUE                     \
    ? BASE64_BAD_CHARACTER                                                  \
    : ((uint32_t)(v) >> 4) | (((uint32_t)(v) & 15U) << 12))
BASE64_DECODE_TABLE_STORAGE uint32_t base64_decode_1[256] = {
    BASE64_DECODE_VALUES(BASE64_TABLE_1)
};
#undef BASE64_TABLE_1

#define BASE64_TABLE_2(v) ((v) == BASE64_INVALID_VALUE                     \
    ? BASE64_BAD_CHARACTER                                                  \
    : (((uint32_t)(v) >> 2) << 8) | (((uint32_t)(v) & 3U) << 22))
BASE64_DECODE_TABLE_STORAGE uint32_t base64_decode_2[256] = {
    BASE64_DECODE_VALUES(BASE64_TABLE_2)
};
#undef BASE64_TABLE_2

#define BASE64_TABLE_3(v) ((v) == BASE64_INVALID_VALUE                     \
    ? BASE64_BAD_CHARACTER : (uint32_t)(v) << 16)
BASE64_DECODE_TABLE_STORAGE uint32_t base64_decode_3[256] = {
    BASE64_DECODE_VALUES(BASE64_TABLE_3)
};
#undef BASE64_TABLE_3
#undef BASE64_DECODE_TABLE_STORAGE

#undef BASE64_DECODE_VALUES
#undef BASE64_REPEAT_8
#undef BASE64_REPEAT_16
#undef BASE64_INVALID_VALUE

#define BASE64_DECODE_VALUE(p)                                              \
    (base64_decode_0[(p)[0]] | base64_decode_1[(p)[1]] |                   \
     base64_decode_2[(p)[2]] | base64_decode_3[(p)[3]])

static inline size_t base64_decode_tail(const unsigned char* input,
                                        size_t length,
                                        unsigned char* output,
                                        unsigned char* begin)
{
    uint32_t value;
    if (length == 0U)
        return (size_t)(output - begin);
    if (length == 1U)
        return BASE64_ERROR;
    value = base64_decode_0[input[0]] | base64_decode_1[input[1]];
    if (length == 3U)
        value |= base64_decode_2[input[2]];
    if (value >= BASE64_BAD_CHARACTER)
        return BASE64_ERROR;
    output[0] = (unsigned char)value;
    if (length == 3U)
        output[1] = (unsigned char)(value >> 8);
    return (size_t)(output - begin) + length - 1U;
}

static inline size_t base64_decode_tail_unchecked(const unsigned char* input,
                                                  size_t length,
                                                  unsigned char* output,
                                                  unsigned char* begin)
{
    uint32_t value;
    if (length == 0U)
        return (size_t)(output - begin);
    if (length == 1U)
        return BASE64_ERROR;
    value = base64_decode_0[input[0]] | base64_decode_1[input[1]];
    if (length == 3U)
        value |= base64_decode_2[input[2]];
    output[0] = (unsigned char)value;
    if (length == 3U)
        output[1] = (unsigned char)(value >> 8);
    return (size_t)(output - begin) + length - 1U;
}

static inline size_t base64_decode_short(const unsigned char* input,
                                         size_t length,
                                         unsigned char* output,
                                         int support_url_safe)
{
    const size_t quartets = length >> 2;
    const size_t remainder = length & 3U;
    uint32_t invalid = 0U;
    uint32_t value;

#define BASE64_DECODE_SHORT_QUARTET(i)                                     \
    do {                                                                    \
        value = BASE64_DECODE_VALUE(input + (i) * 4U);                     \
        if (!support_url_safe &&                                           \
            (input[(i) * 4U] == '-' || input[(i) * 4U] == '_' ||          \
             input[(i) * 4U + 1U] == '-' || input[(i) * 4U + 1U] == '_' ||\
             input[(i) * 4U + 2U] == '-' || input[(i) * 4U + 2U] == '_' ||\
             input[(i) * 4U + 3U] == '-' || input[(i) * 4U + 3U] == '_')) \
            value |= BASE64_BAD_CHARACTER;                                \
        invalid |= value;                                                   \
        output[(i) * 3U] = (unsigned char)value;                           \
        output[(i) * 3U + 1U] = (unsigned char)(value >> 8);               \
        output[(i) * 3U + 2U] = (unsigned char)(value >> 16);              \
    } while (0)

    switch (quartets) {
    case 6: BASE64_DECODE_SHORT_QUARTET(5); /* fall through */
    case 5: BASE64_DECODE_SHORT_QUARTET(4); /* fall through */
    case 4: BASE64_DECODE_SHORT_QUARTET(3); /* fall through */
    case 3: BASE64_DECODE_SHORT_QUARTET(2); /* fall through */
    case 2: BASE64_DECODE_SHORT_QUARTET(1); /* fall through */
    case 1: BASE64_DECODE_SHORT_QUARTET(0); /* fall through */
    default: break;
    }
#undef BASE64_DECODE_SHORT_QUARTET

    if (invalid >= BASE64_BAD_CHARACTER)
        return BASE64_ERROR;
    input += quartets * 4U;
    output += quartets * 3U;
    if (remainder == 0U)
        return quartets * 3U;
    if (remainder == 1U)
        return BASE64_ERROR;
    value = base64_decode_0[input[0]] | base64_decode_1[input[1]];
    if (remainder == 3U)
        value |= base64_decode_2[input[2]];
    if (!support_url_safe &&
        (input[0] == '-' || input[0] == '_' ||
         input[1] == '-' || input[1] == '_' ||
         (remainder == 3U && (input[2] == '-' || input[2] == '_'))))
        value |= BASE64_BAD_CHARACTER;
    if (value >= BASE64_BAD_CHARACTER)
        return BASE64_ERROR;
    output[0] = (unsigned char)value;
    if (remainder == 3U)
        output[1] = (unsigned char)(value >> 8);
    return quartets * 3U + remainder - 1U;
}

static inline size_t base64_decode_short_unchecked(
    const unsigned char* input, size_t length, unsigned char* output,
    int support_url_safe)
{
    const size_t quartets = length >> 2;
    const size_t remainder = length & 3U;
    uint32_t invalid_url = 0U;
    uint32_t value;

#define BASE64_DECODE_SHORT_UNCHECKED_QUARTET(i)                            \
    do {                                                                    \
        value = BASE64_DECODE_VALUE(input + (i) * 4U);                     \
        if (!support_url_safe)                                              \
            invalid_url |=                                                  \
                input[(i) * 4U] == '-' || input[(i) * 4U] == '_' ||        \
                input[(i) * 4U + 1U] == '-' || input[(i) * 4U + 1U] == '_' || \
                input[(i) * 4U + 2U] == '-' || input[(i) * 4U + 2U] == '_' || \
                input[(i) * 4U + 3U] == '-' || input[(i) * 4U + 3U] == '_'; \
        output[(i) * 3U] = (unsigned char)value;                           \
        output[(i) * 3U + 1U] = (unsigned char)(value >> 8);               \
        output[(i) * 3U + 2U] = (unsigned char)(value >> 16);              \
    } while (0)

    switch (quartets) {
    case 6: BASE64_DECODE_SHORT_UNCHECKED_QUARTET(5); /* fall through */
    case 5: BASE64_DECODE_SHORT_UNCHECKED_QUARTET(4); /* fall through */
    case 4: BASE64_DECODE_SHORT_UNCHECKED_QUARTET(3); /* fall through */
    case 3: BASE64_DECODE_SHORT_UNCHECKED_QUARTET(2); /* fall through */
    case 2: BASE64_DECODE_SHORT_UNCHECKED_QUARTET(1); /* fall through */
    case 1: BASE64_DECODE_SHORT_UNCHECKED_QUARTET(0); /* fall through */
    default: break;
    }
#undef BASE64_DECODE_SHORT_UNCHECKED_QUARTET

    input += quartets * 4U;
    output += quartets * 3U;
    if (remainder == 0U)
        return invalid_url ? BASE64_ERROR : quartets * 3U;
    value = base64_decode_0[input[0]] | base64_decode_1[input[1]];
    if (remainder == 3U)
        value |= base64_decode_2[input[2]];
    if (!support_url_safe)
        invalid_url |= input[0] == '-' || input[0] == '_' ||
                       input[1] == '-' || input[1] == '_' ||
                       (remainder == 3U &&
                        (input[2] == '-' || input[2] == '_'));
    if (invalid_url)
        return BASE64_ERROR;
    output[0] = (unsigned char)value;
    if (remainder == 3U)
        output[1] = (unsigned char)(value >> 8);
    return quartets * 3U + remainder - 1U;
}

BASE64_API size_t base64_decode(const unsigned char* BASE64_RESTRICT input,
                                size_t length,
                                unsigned char* BASE64_RESTRICT output,
                                int support_url_safe)
{
    unsigned char* const begin = output;
    size_t data_length = length;
    size_t padding = 0U;

    if (data_length != 0U && input[data_length - 1U] == '=') {
        --data_length;
        ++padding;
        if (data_length != 0U && input[data_length - 1U] == '=') {
            --data_length;
            ++padding;
        }
    }
    if (padding != 0U) {
        const size_t expected_remainder = padding == 1U ? 3U : 2U;
        if ((length & 3U) != 0U ||
            data_length % 4U != expected_remainder)
            return BASE64_ERROR;
    }
    else if (data_length % 4U == 1U) {
        return BASE64_ERROR;
    }

    if (data_length <= 24U)
        return base64_decode_short(input, data_length, output,
                                   support_url_safe);
    if (!support_url_safe &&
        (memchr(input, '-', data_length) != NULL ||
         memchr(input, '_', data_length) != NULL))
        return BASE64_ERROR;

    while (data_length >= 12U) {
        const uint32_t x0 = BASE64_DECODE_VALUE(input);
        const uint32_t x1 = BASE64_DECODE_VALUE(input + 4);
        const uint32_t x2 = BASE64_DECODE_VALUE(input + 8);
        if ((x0 | x1 | x2) >= BASE64_BAD_CHARACTER)
            return BASE64_ERROR;
#if defined(WORDS_BIGENDIAN) || \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
        output[0] = (unsigned char)x0;
        output[1] = (unsigned char)(x0 >> 8);
        output[2] = (unsigned char)(x0 >> 16);
        output[3] = (unsigned char)x1;
        output[4] = (unsigned char)(x1 >> 8);
        output[5] = (unsigned char)(x1 >> 16);
        output[6] = (unsigned char)x2;
        output[7] = (unsigned char)(x2 >> 8);
        output[8] = (unsigned char)(x2 >> 16);
#else
        {
            const uint64_t word =
                (uint64_t)(x0 & 0x00FFFFFFU) |
                ((uint64_t)(x1 & 0x00FFFFFFU) << 24) |
                ((uint64_t)(x2 & 0xFFFFU) << 48);
            memcpy(output, &word, 8);
            output[8] = (unsigned char)(x2 >> 16);
        }
#endif
        input += 12;
        output += 9;
        data_length -= 12;
    }
    while (data_length >= 4U) {
        const uint32_t value = BASE64_DECODE_VALUE(input);
        if (value >= BASE64_BAD_CHARACTER)
            return BASE64_ERROR;
        output[0] = (unsigned char)value;
        output[1] = (unsigned char)(value >> 8);
        output[2] = (unsigned char)(value >> 16);
        input += 4;
        output += 3;
        data_length -= 4;
    }
    return base64_decode_tail(input, data_length, output, begin);
}

BASE64_API size_t base64_decode_unchecked(
    const unsigned char* BASE64_RESTRICT input,
    size_t length,
    unsigned char* BASE64_RESTRICT output,
    int support_url_safe)
{
    unsigned char* const begin = output;
    size_t data_length = length;
    size_t padding = 0U;

    if (data_length != 0U && input[data_length - 1U] == '=') {
        --data_length;
        ++padding;
        if (data_length != 0U && input[data_length - 1U] == '=') {
            --data_length;
            ++padding;
        }
    }
    if (padding != 0U) {
        const size_t expected_remainder = padding == 1U ? 3U : 2U;
        if ((length & 3U) != 0U ||
            data_length % 4U != expected_remainder)
            return BASE64_ERROR;
    }
    else if (data_length % 4U == 1U) {
        return BASE64_ERROR;
    }

    if (data_length <= 24U)
        return base64_decode_short_unchecked(input, data_length, output,
                                             support_url_safe);
    if (!support_url_safe &&
        (memchr(input, '-', data_length) != NULL ||
         memchr(input, '_', data_length) != NULL))
        return BASE64_ERROR;

    while (data_length >= 12U) {
        const uint32_t x0 = BASE64_DECODE_VALUE(input);
        const uint32_t x1 = BASE64_DECODE_VALUE(input + 4);
        const uint32_t x2 = BASE64_DECODE_VALUE(input + 8);
#if defined(WORDS_BIGENDIAN) || \
    (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
        output[0] = (unsigned char)x0;
        output[1] = (unsigned char)(x0 >> 8);
        output[2] = (unsigned char)(x0 >> 16);
        output[3] = (unsigned char)x1;
        output[4] = (unsigned char)(x1 >> 8);
        output[5] = (unsigned char)(x1 >> 16);
        output[6] = (unsigned char)x2;
        output[7] = (unsigned char)(x2 >> 8);
        output[8] = (unsigned char)(x2 >> 16);
#else
        {
            const uint64_t word =
                (uint64_t)(x0 & 0x00FFFFFFU) |
                ((uint64_t)(x1 & 0x00FFFFFFU) << 24) |
                ((uint64_t)(x2 & 0xFFFFU) << 48);
            memcpy(output, &word, 8);
            output[8] = (unsigned char)(x2 >> 16);
        }
#endif
        input += 12;
        output += 9;
        data_length -= 12;
    }
    while (data_length >= 4U) {
        const uint32_t value = BASE64_DECODE_VALUE(input);
        output[0] = (unsigned char)value;
        output[1] = (unsigned char)(value >> 8);
        output[2] = (unsigned char)(value >> 16);
        input += 4;
        output += 3;
        data_length -= 4;
    }
    if (data_length == 0U)
        return (size_t)(output - begin);
    {
        uint32_t value = base64_decode_0[input[0]] |
                         base64_decode_1[input[1]];
        if (data_length == 3U)
            value |= base64_decode_2[input[2]];
        output[0] = (unsigned char)value;
        if (data_length == 3U)
            output[1] = (unsigned char)(value >> 8);
    }
    return (size_t)(output - begin) + data_length - 1U;
}

#undef BASE64_DECODE_VALUE
#undef BASE64_BAD_CHARACTER
#undef BASE64URL_ENCODE_PAIR
#undef BASE64_ENCODE_PAIR
#undef BASE64_RESTRICT

#endif /* BASE64_INCLUDE_IMPLEMENTATION */

#if !defined(BASE64_IMPLEMENTATION) && !defined(BASE64_HEADER_ONLY)
#if defined(_MSC_VER)
#  define BASE64_ALWAYS_INLINE static __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#  define BASE64_ALWAYS_INLINE static inline __attribute__((always_inline))
#else
#  define BASE64_ALWAYS_INLINE static inline
#endif

BASE64_ALWAYS_INLINE size_t base64_decode_inline_short(
    const unsigned char* input, size_t length, unsigned char* output,
    int support_url_safe, int checked)
{
    size_t data_length = length;
    size_t padding = 0U;
    size_t quartets, remainder;
    uint32_t invalid = 0U, value;
    if (data_length && input[data_length - 1U] == '=') {
        --data_length; ++padding;
        if (data_length && input[data_length - 1U] == '=') {
            --data_length; ++padding;
        }
    }
    if ((padding && ((length & 3U) != 0U ||
         data_length % 4U != (padding == 1U ? 3U : 2U))) ||
        (!padding && data_length % 4U == 1U))
        return BASE64_ERROR;
    quartets = data_length >> 2;
    remainder = data_length & 3U;
#define BASE64_INLINE_QUARTET(i)                                           \
    do {                                                                    \
        const unsigned char* p = input + (i) * 4U;                         \
        value = base64_decode_0[p[0]] | base64_decode_1[p[1]] |            \
                base64_decode_2[p[2]] | base64_decode_3[p[3]];             \
        if (checked) invalid |= value;                                      \
        if (!support_url_safe &&                                           \
            (p[0] == '-' || p[0] == '_' || p[1] == '-' || p[1] == '_' || \
             p[2] == '-' || p[2] == '_' || p[3] == '-' || p[3] == '_'))  \
            invalid |= 0x01FFFFFFU;                                        \
        output[(i) * 3U] = (unsigned char)value;                           \
        output[(i) * 3U + 1U] = (unsigned char)(value >> 8);               \
        output[(i) * 3U + 2U] = (unsigned char)(value >> 16);              \
    } while (0)
    switch (quartets) {
    case 5: BASE64_INLINE_QUARTET(4); /* fall through */
    case 4: BASE64_INLINE_QUARTET(3); /* fall through */
    case 3: BASE64_INLINE_QUARTET(2); /* fall through */
    case 2: BASE64_INLINE_QUARTET(1); /* fall through */
    case 1: BASE64_INLINE_QUARTET(0); /* fall through */
    default: break;
    }
#undef BASE64_INLINE_QUARTET
    if (invalid >= 0x01FFFFFFU)
        return BASE64_ERROR;
    input += quartets * 4U;
    output += quartets * 3U;
    if (!remainder)
        return quartets * 3U;
    value = base64_decode_0[input[0]] | base64_decode_1[input[1]];
    if (remainder == 3U) value |= base64_decode_2[input[2]];
    if (checked && value >= 0x01FFFFFFU)
        return BASE64_ERROR;
    if (!support_url_safe &&
        (input[0] == '-' || input[0] == '_' || input[1] == '-' ||
         input[1] == '_' || (remainder == 3U &&
         (input[2] == '-' || input[2] == '_'))))
        return BASE64_ERROR;
    output[0] = (unsigned char)value;
    if (remainder == 3U) output[1] = (unsigned char)(value >> 8);
    return quartets * 3U + remainder - 1U;
}

BASE64_ALWAYS_INLINE size_t base64_decode(const unsigned char* input,
    size_t length, unsigned char* output, int support_url_safe)
{
    return length <= 20U
        ? base64_decode_inline_short(input, length, output, support_url_safe, 1)
        : base64_decode_compiled(input, length, output, support_url_safe);
}

BASE64_ALWAYS_INLINE size_t base64_decode_unchecked(const unsigned char* input,
    size_t length, unsigned char* output, int support_url_safe)
{
    return length <= 20U
        ? base64_decode_inline_short(input, length, output, support_url_safe, 0)
        : base64_decode_unchecked_compiled(input, length, output,
                                           support_url_safe);
}
#undef BASE64_ALWAYS_INLINE
#endif

#undef BASE64_INCLUDE_IMPLEMENTATION
#undef BASE64_API

#endif /* BASE64_H_INCLUDED */
