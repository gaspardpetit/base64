#include "omnifarious.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct omnifarious
{
	std::string encode(std::string &bytes)
	{
		return base64_encode(bytes.data(), bytes.length());
	}
};

IMPLEMENT_TESTS(omnifarious);

