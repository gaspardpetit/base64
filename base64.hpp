/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright (c) 2026 Gaspard Petit
 */

#ifndef BASE64_HPP_INCLUDED
#define BASE64_HPP_INCLUDED

#if defined(BASE64_CPP_HEADER_ONLY) && !defined(BASE64_HEADER_ONLY)
#  define BASE64_HEADER_ONLY
#  define BASE64_HPP_DEFINED_HEADER_ONLY
#endif

#include "base64.h"

#ifdef BASE64_HPP_DEFINED_HEADER_ONLY
#  undef BASE64_HEADER_ONLY
#  undef BASE64_HPP_DEFINED_HEADER_ONLY
#endif

#include <stdexcept>
#include <string>

namespace base64 {

#if !defined(BASE64_CPP_HEADER_ONLY) && !defined(BASE64_CPP_IMPLEMENTATION)

std::string encode(const std::string& input);
std::string url_encode(const std::string& input);
std::string decode(const std::string& input);
void encode(const std::string& input, std::string& output);
void url_encode(const std::string& input, std::string& output);
bool decode(const std::string& input, std::string& output);

#else

#if defined(BASE64_CPP_IMPLEMENTATION)
#  define BASE64_CPP_API
#else
#  define BASE64_CPP_API inline
#endif

BASE64_CPP_API void encode(const std::string& input, std::string& output)
{
    if (&input == &output)
        throw std::invalid_argument("Base64 input and output must be distinct");
    output.resize(base64_encoded_size(input.size()));
    base64_encode(
        reinterpret_cast<const unsigned char*>(input.data()),
        input.size(),
        output.data());
}

BASE64_CPP_API void url_encode(const std::string& input, std::string& output)
{
    if (&input == &output)
        throw std::invalid_argument("Base64 input and output must be distinct");
    output.resize(base64_encoded_size(input.size()));
    base64url_encode(
        reinterpret_cast<const unsigned char*>(input.data()),
        input.size(),
        output.data());
}

BASE64_CPP_API bool decode(const std::string& input, std::string& output)
{
    if (&input == &output)
        throw std::invalid_argument("Base64 input and output must be distinct");
    output.resize(base64_decoded_max_size(input.size()));
    const size_t size = base64_decode(
        reinterpret_cast<const unsigned char*>(input.data()),
        input.size(),
        reinterpret_cast<unsigned char*>(output.data()));
    if (size == BASE64_ERROR) {
        output.clear();
        return false;
    }
    output.resize(size);
    return true;
}

BASE64_CPP_API std::string encode(const std::string& input)
{
    std::string output;
    encode(input, output);
    return output;
}

BASE64_CPP_API std::string url_encode(const std::string& input)
{
    std::string output;
    url_encode(input, output);
    return output;
}

BASE64_CPP_API std::string decode(const std::string& input)
{
    std::string output;
    if (!decode(input, output))
        throw std::invalid_argument("Invalid Base64 input");
    return output;
}

#undef BASE64_CPP_API

#endif

} /* namespace base64 */

#endif /* BASE64_HPP_INCLUDED */
