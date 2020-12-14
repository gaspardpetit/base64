#include "chromium/modp_b64/modp_b64.h"
#include <iostream>
#include <gtest/gtest.h>
#include "../test_base.hpp"
#include <string>

using namespace std;

struct B64_Adapt
{
	static constexpr const size_t GetEncodedLengthWithoutPadding(size_t decodedLength)
	{
		return (decodedLength * 8 + 5) / 6;
	}

	static std::string encode(size_t(*func)(char*,const char*,size_t), const std::string &bytes)
	{
		size_t encLen = GetEncodedLengthWithoutPadding(bytes.length());
		std::string encoded(encLen, '=');
		func((char*)&encoded[0], (const char*)&bytes[0], bytes.length());
		return encoded;
	}
};

struct modp_b64_chromium
{
	std::string encode(std::string &bytes)
	{
		return B64_Adapt::encode(modp_b64_encode, bytes);
	}

	std::string decode(std::string &bytes)
	{
		return modp_b64_decode(bytes);
	}
};

IMPLEMENT_ENCODE_TESTS(modp_b64_chromium);
IMPLEMENT_DECODE_TESTS(modp_b64_chromium);
