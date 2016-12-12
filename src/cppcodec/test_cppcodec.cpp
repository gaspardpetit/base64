#include "cppcodec/base64_default_rfc4648.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct CppCodec
{
	std::string encode(std::string &bytes)
	{
		return base64::encode((unsigned char*)bytes.data(), bytes.length());
	}
};

IMPLEMENT_TESTS(CppCodec);
