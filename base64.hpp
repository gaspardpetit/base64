#ifndef BASE64_HPP_INCLUDED
#define BASE64_HPP_INCLUDED

#if defined(BASE64_CPP_COMPILED) && !defined(BASE64_COMPILED)
#  define BASE64_COMPILED
#  define BASE64_HPP_DEFINED_COMPILED
#endif

#include "base64.h"

#ifdef BASE64_HPP_DEFINED_COMPILED
#  undef BASE64_COMPILED
#  undef BASE64_HPP_DEFINED_COMPILED
#endif

#include <stdexcept>
#include <string>

namespace base64 {

#if defined(BASE64_CPP_COMPILED) && !defined(BASE64_CPP_BUILD)

std::string encode(const std::string& input);
std::string decode(const std::string& input);

#else

#if defined(BASE64_CPP_BUILD)
#  define BASE64_CPP_API
#else
#  define BASE64_CPP_API inline
#endif

BASE64_CPP_API std::string encode(const std::string& input)
{
    std::string output(base64_encoded_size(input.size()), '\0');
    base64_encode(
        reinterpret_cast<const unsigned char*>(input.data()),
        input.size(),
        output.data());
    return output;
}

BASE64_CPP_API std::string decode(const std::string& input)
{
    std::string output(base64_decoded_max_size(input.size()), '\0');
    const size_t size = base64_decode(
        reinterpret_cast<const unsigned char*>(input.data()),
        input.size(),
        reinterpret_cast<unsigned char*>(output.data()));
    if (size == BASE64_ERROR)
        throw std::invalid_argument("Invalid Base64 input");
    output.resize(size);
    return output;
}

#undef BASE64_CPP_API

#endif

} /* namespace base64 */

#endif /* BASE64_HPP_INCLUDED */
