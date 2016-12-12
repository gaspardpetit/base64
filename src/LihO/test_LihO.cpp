#include "LihO.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct LihO
{
	std::string encode(std::string &bytes)
	{
		return LihO_base64_encode((unsigned char*)bytes.data(), bytes.length());
	}
};

IMPLEMENT_TESTS(LihO);
