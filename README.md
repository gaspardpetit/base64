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

## C++

Compile `base64.cpp` and include the header:

```cpp
#include "base64.hpp"

std::string encoded = base64::encode("Hello");
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

These overloads do not allocate when the output string already has sufficient
capacity. Input and output must be distinct strings.

## Supported format

This library implements standard padded Base64 as defined by RFC 4648. It does
not accept the URL-safe alphabet, whitespace, or line-wrapped input. Input and
output buffers must not overlap.

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

The implementation was developed through experiments in the
[base64-benchmark](https://github.com/gaspardpetit/base64-benchmark) project and
was informed by the table-driven and unrolled approaches used by
[Chromium's `modp_b64`](https://chromium.googlesource.com/chromium/src/third_party/modp_b64/)
and [TurboBase64](https://github.com/powturbo/Turbo-Base64). It does not require
architecture-specific intrinsics.

## License

This project is available under the [BSD 3-Clause License](LICENSE).
