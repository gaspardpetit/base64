#include "../Base64SurveyRegistry.hpp"
#include <gtest/gtest.h>
#include "singleheader-6.3.1/simdutf.h"
#include "singleheader-6.3.1/simdutf.cpp"


struct simdutfb64
{
  std::string encode(const std::string& bytes)
  {
    // There is no string constructor with uninitialised values and resize uses value-initialized characters
    std::string buffer(simdutf::base64_length_from_binary(bytes.size()),char(0));
    simdutf::binary_to_base64(bytes.data(), bytes.size(), buffer.data());
    return buffer;
  }
  
  std::string decode(const std::string& base64)
  {
    // There is no string constructor with uninitialised values and resize uses value-initialized characters
    std::string buffer(simdutf::maximal_binary_length_from_base64(base64.data(), base64.size()),char(0));
    simdutf::result r = simdutf::base64_to_binary(base64.data(), base64.size(), buffer.data());
    if(r.error) {
      // We have some error, r.count tells you where the error was encountered in the input if
      // the error is INVALID_BASE64_CHARACTER. If the error is BASE64_INPUT_REMAINDER, then
      // a single valid base64 character remained, and r.count contains the number of bytes decoded.
      buffer.resize(0);
    } else {
      buffer.resize(r.count); // resize the buffer according to actual number of bytes
    }
    return buffer;
  }
};

BASE64_REGISTER_ENCODER(simdutfb64);
BASE64_REGISTER_DECODER(simdutfb64);
