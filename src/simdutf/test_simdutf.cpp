#include "../Base64SurveyRegistry.hpp"
#include <gtest/gtest.h>
#include "simdutf.h"
#include "simdutf.cpp"


struct simdutfb64
{
  std::string encode(const std::string& bytes)
  {
    // For some reason working directly with a std::string leads to unit test failure
    std::vector<char> buffer(simdutf::base64_length_from_binary(bytes.size()));
    simdutf::binary_to_base64(bytes.data(), bytes.size(), buffer.data());
    return std::string(buffer.begin(), buffer.end());
  }
  
  std::string decode(const std::string& base64)
  {
    // For some reason working directly with a std::string leads to unit test failure
    std::vector<char> buffer(simdutf::maximal_binary_length_from_base64(base64.data(), base64.size()));
    simdutf::result r = simdutf::base64_to_binary(base64.data(), base64.size(), buffer.data());
    if(r.error) {
      // We have some error, r.count tells you where the error was encountered in the input if
      // the error is INVALID_BASE64_CHARACTER. If the error is BASE64_INPUT_REMAINDER, then
      // a single valid base64 character remained, and r.count contains the number of bytes decoded.
      buffer.resize(0);
    } else {
      buffer.resize(r.count); // resize the buffer according to actual number of bytes
    }
    return std::string(buffer.begin(), buffer.end());
  }
};

BASE64_REGISTER_ENCODER(simdutfb64);
BASE64_REGISTER_DECODER(simdutfb64);
