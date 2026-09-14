# base64

An efficient, portable Base64 encoder and decoder for C and C++. At a 32 KiB
payload size, it is the fastest architecture-neutral encoder and decoder in the
current Linux, Windows, and macOS results from
[base64-benchmark](https://github.com/gaspardpetit/base64-benchmark);
implementations using architecture-specific SIMD instructions are considered
separately.

## C

Compile `base64.c` and include the header:

```c
#include "base64.h"
```

For header-only use, define `BASE64_HEADER_ONLY` before including the header:

```c
#define BASE64_HEADER_ONLY
#include "base64.h"
```

Allocate `base64_encoded_size(input_size)` bytes for encoding or
`base64_decoded_max_size(input_size)` bytes for decoding. The functions
return the number of bytes written. `base64_decode` returns `BASE64_ERROR`
for invalid input.

Use `base64url_encode` to produce URL-safe Base64. It has the same buffer-size
and return-value contract as `base64_encode`.

`base64_decode_unchecked` skips character validation when the input is already
trusted. It remains bounds-safe and checks the encoded length and padding, but
invalid alphabet characters produce unspecified decoded bytes.

## C++

Compile `base64.cpp` and include the header:

```cpp
#include "base64.hpp"

std::string encoded = base64::encode("Hello");
std::string url_encoded = base64::url_encode("Hello");
std::string decoded = base64::decode(encoded);
```

For header-only use, define `BASE64_CPP_HEADER_ONLY` before including the
header:

```cpp
#define BASE64_CPP_HEADER_ONLY
#include "base64.hpp"
```

To reuse caller-owned storage:

```cpp
std::string encoded;
encoded.reserve(base64_encoded_size(input.size()));
base64::encode(input, encoded);

std::string decoded;
decoded.reserve(base64_decoded_max_size(encoded.size()));
if (!base64::decode(encoded, decoded)) {
    // Invalid Base64 input.
}
```

For trusted input, `base64::decode_unchecked` provides returning-string and
caller-owned-output overloads equivalent to `base64::decode`.

These overloads do not allocate when the output string already has sufficient
capacity. Input and output must be distinct strings.

## Supported format

`base64_encode` produces standard padded Base64 as defined by RFC 4648;
`base64url_encode` produces its padded URL-safe variant. Decoding accepts both
the standard (`+` and `/`) and URL-safe (`-` and `_`) alphabets, with or without
padding. Mixed alphabets are also accepted. Whitespace and line-wrapped input
are rejected. Input and output buffers must not overlap.

## Linux AVX2

Compiled-library builds use runtime AVX2 dispatch on x86-64 with GCC or Clang.
Compile the backend separately so the public API and scalar fallback remain
usable on processors without AVX2:

```sh
clang -O3 -c base64.c
clang -O3 -mavx2 -c base64_avx2.c
```

Link both objects. CPU detection includes OS support for AVX register state.
Define `BASE64_DISABLE_HARDWARE` when compiling `base64.c` to omit the backend.
Header-only builds remain scalar.

## Windows AVX2

Compiled-library builds use runtime AVX2 dispatch by default on Windows x64.
Compile `base64_avx2.c` separately with `/arch:AVX2` and link both objects:

```bat
cl /O2 /c base64.c
cl /O2 /c /arch:AVX2 base64_avx2.c
```

For C++, compile `base64.cpp` instead of `base64.c`. The public functions detect
AVX2 once and otherwise use the portable scalar implementation.

Define `BASE64_DISABLE_HARDWARE` when compiling `base64.c` or `base64.cpp` to
build only the portable implementation without linking `base64_avx2.c`.
Header-only builds remain scalar and do not require this definition.

## Apple NEON

Compiled-library builds use the NEON backend by default on Apple Silicon and
other AArch64 targets. Compile `base64_neon.c` and link both objects:

```sh
clang -O3 -c base64.c
clang -O3 -c base64_neon.c
```

For C++, compile `base64.cpp` instead of `base64.c`. The backend is selected at
compile time because NEON is required by AArch64. Define
`BASE64_DISABLE_HARDWARE` when compiling `base64.c` or `base64.cpp` to build
only the portable implementation without linking `base64_neon.c`. Header-only
builds remain scalar and do not require this definition.

## Technical overview

The C API writes directly to caller-provided buffers and performs no dynamic
memory allocation. The C++ output overloads resize an existing `std::string`,
allowing callers to reserve and reuse its storage.

Encoding uses a 64 × 64 lookup table that converts 12 input bits into two
Base64 characters at once. The main loop processes 12 input bytes per
iteration, reducing loop overhead and independent table lookups while remaining
portable C.

Decoding uses four position-specific 256-entry tables. Each lookup contributes
the decoded bits in their final position, allowing four input characters to be
combined with bitwise OR operations while detecting invalid characters. The
main loop processes 12 encoded characters at a time and writes the resulting
nine bytes with packed stores where the platform permits it.

The optional AVX2 backend processes four 24-byte encoding blocks together and
three 32-byte decoding blocks per iteration. Decoding uses independently
derived hash tables to translate and validate both RFC 4648 alphabets, followed
by packed multiply-add operations that assemble the decoded bytes. Checked and
unchecked decoding use separately specialized loops.

The optional NEON backend processes 48 input bytes per encoding block and 64
Base64 characters per decoding block. Its checked decoder uses compact lookup
tables to validate and translate the standard alphabet in one pass; URL-safe
input falls back to the scalar decoder. The unchecked variant skips validation
for trusted standard-Base64 input and uses the scalar path for URL-safe input.

The implementation was developed through experiments in the
[base64-benchmark](https://github.com/gaspardpetit/base64-benchmark) project and
was informed by the table-driven and unrolled approaches used by
[Chromium's `modp_b64`](https://chromium.googlesource.com/chromium/src/third_party/modp_b64/)
and [TurboBase64](https://github.com/powturbo/Turbo-Base64). The portable path
does not require architecture-specific intrinsics.

## License

This project is available under the [BSD 3-Clause License](LICENSE).
