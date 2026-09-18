/* SPDX-License-Identifier: BSD-3-Clause */

#include "base64.h"

#include <stdio.h>
#include <string.h>

static int test_compact(void)
{
    size_t length;
    for (length = 0U; length <= 257U; ++length) {
        unsigned char buffer[260];
        unsigned char expected[260];
        size_t expected_size = 0U;
        size_t i;
        memset(buffer, 0xA5, sizeof(buffer));
        for (i = 0U; i < length; ++i) {
            const unsigned char byte = (unsigned char)(i * 73U + length);
            buffer[i + 1U] = byte;
            if (byte > 0x20U)
                expected[expected_size++] = byte;
        }
        if (base64_compact(buffer + 1U, length) != expected_size ||
            memcmp(buffer + 1U, expected, expected_size) != 0) {
            fprintf(stderr, "compact failed at length %zu\n", length);
            return 0;
        }
    }
    {
        size_t offset;
        for (offset = 0U; offset != 16U; ++offset) {
            unsigned int mask;
            for (mask = 0U; mask != 256U; ++mask) {
                unsigned char buffer[128];
                unsigned char expected[96];
                const size_t input_size = 64U + (size_t)(mask & 31U);
                size_t expected_size = 0U;
                size_t i;
                memset(buffer, 0xA5, sizeof(buffer));
                for (i = 0U; i != input_size; ++i) {
                    const unsigned char byte = (mask & (1U << (i & 7U)))
                        ? (unsigned char)(0x21U + (i * 29U) % 0xDEU)
                        : (unsigned char)(i % 0x21U);
                    buffer[offset + i] = byte;
                    if (byte > 0x20U)
                        expected[expected_size++] = byte;
                }
                if (base64_compact(buffer + offset, input_size) != expected_size ||
                    memcmp(buffer + offset, expected, expected_size) != 0 ||
                    (offset != 0U && buffer[offset - 1U] != 0xA5U) ||
                    buffer[offset + input_size] != 0xA5U) {
                    fprintf(stderr,
                            "compact mask/alignment failed at %u/%zu\n",
                            mask, offset);
                    return 0;
                }
            }
        }
    }
    return 1;
}

static int test_decode_whitespace(void)
{
    unsigned char encoded[] = "Zm9v\0\r\n\t YmFy";
    unsigned char decoded[16];
    const size_t size = base64_decode_whitespace(
        encoded, sizeof(encoded) - 1U, decoded);
    if (size != 6U || memcmp(decoded, "foobar", 6U) != 0)
        return 0;
    {
        unsigned char invalid[] = "Zm!9v";
        return base64_decode_whitespace(
            invalid, sizeof(invalid) - 1U, decoded) == BASE64_ERROR;
    }
}

static int test_decode_alphabet_modes(void)
{
    static const unsigned char standard[] = "AAA+AAA/";
    static const unsigned char url_safe[] = "AAA-AAA_";
    unsigned char output[8];
    return base64_decode(standard, sizeof(standard) - 1U, output) == 6U &&
           base64url_decode(standard, sizeof(standard) - 1U, output) == 6U &&
           base64_decode(url_safe, sizeof(url_safe) - 1U, output) ==
               BASE64_ERROR &&
           base64url_decode(url_safe, sizeof(url_safe) - 1U, output) == 6U;
}

int main(void)
{
    const char* backend = base64_runtime_backend();
    if (strcmp(backend, "AVX2") != 0 && strcmp(backend, "NEON") != 0 &&
        strcmp(backend, "scalar") != 0) {
        fprintf(stderr, "unexpected backend: %s\n", backend);
        return 1;
    }
    if (!test_compact() || !test_decode_whitespace() ||
        !test_decode_alphabet_modes()) {
        fprintf(stderr, "Base64 API test failed\n");
        return 1;
    }
    return 0;
}
