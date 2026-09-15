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
    return 1;
}

static int test_decode_whitespace(void)
{
    unsigned char encoded[] = "Zm9v\0\r\n\t YmFy";
    unsigned char decoded[16];
    const size_t size = base64_decode_whitespace(
        encoded, sizeof(encoded) - 1U, decoded, 0);
    if (size != 6U || memcmp(decoded, "foobar", 6U) != 0)
        return 0;
    {
        unsigned char invalid[] = "Zm!9v";
        return base64_decode_whitespace(
            invalid, sizeof(invalid) - 1U, decoded, 0) == BASE64_ERROR;
    }
}

int main(void)
{
    const char* backend = base64_runtime_backend();
    if (strcmp(backend, "AVX2") != 0 && strcmp(backend, "NEON") != 0 &&
        strcmp(backend, "scalar") != 0) {
        fprintf(stderr, "unexpected backend: %s\n", backend);
        return 1;
    }
    if (!test_compact() || !test_decode_whitespace()) {
        fprintf(stderr, "Base64 API test failed\n");
        return 1;
    }
    return 0;
}
