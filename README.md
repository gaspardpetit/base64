# base64

A small Base64 encoder and decoder for C and C++.

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
