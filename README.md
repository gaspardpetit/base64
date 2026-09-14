# base64

A small Base64 encoder and decoder for C and C++.

## C

For header-only use:

```c
#include "base64.h"
```

To compile the implementation once, compile `base64.c` and define
`BASE64_COMPILED` before including the header in consumers:

```c
#define BASE64_COMPILED
#include "base64.h"
```

Allocate `base64_encoded_size(input_size)` bytes for encoding or
`base64_decoded_max_size(input_size)` bytes for decoding. The functions
return the number of bytes written. `base64_decode` returns `BASE64_ERROR`
for invalid input.

## C++

For header-only use:

```cpp
#include "base64.hpp"

std::string encoded = base64::encode("Hello");
std::string decoded = base64::decode(encoded);
```

To compile the implementation once, compile `base64.cpp` and define
`BASE64_CPP_COMPILED` before including `base64.hpp` in consumers.
