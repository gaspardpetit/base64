#include "LihO.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct LihO
{
	std::string encode(std::string &bytes)
	{
		return LihO_base64_encode((unsigned char*)bytes.data(), bytes.length());
	}
	std::string decode(std::string &bytes)
	{
		return LihO_base64_decode(bytes);
	}
};

IMPLEMENT_ENCODE_TESTS(LihO);
IMPLEMENT_DECODE_TESTS(LihO);
