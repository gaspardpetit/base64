#include <gtest/gtest.h>
#include "../test_base.hpp"

namespace {
#include "ReneNyffenegger/cpp-base64/base64.cpp"
}

struct adp_gmbh
{
	std::string encode(std::string &bytes)
	{
		return base64_encode((unsigned char*)bytes.data(), bytes.length());
	}

	std::string decode(std::string &bytes)
	{
		return base64_decode(bytes);
	}
};

IMPLEMENT_ENCODE_TESTS(adp_gmbh);
IMPLEMENT_DECODE_TESTS(adp_gmbh);
