/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 Gaspard Petit
 */

#ifndef BASE64_AVX2_H_INCLUDED
#define BASE64_AVX2_H_INCLUDED

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t base64_avx2_encode(const unsigned char*, size_t, char*);
size_t base64url_avx2_encode(const unsigned char*, size_t, char*);
size_t base64_avx2_decode(const unsigned char*, size_t, unsigned char*);
size_t base64_avx2_decode_unchecked(const unsigned char*, size_t,
                                    unsigned char*);
size_t base64_avx2_decode_standard(const unsigned char*, size_t,
                                   unsigned char*);
size_t base64_avx2_decode_standard_unchecked(const unsigned char*, size_t,
                                             unsigned char*);

#ifdef __cplusplus
}
#endif

#endif
