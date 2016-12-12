#include "DaedalusAlpha.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct daedalusalpha
{
	std::string encode(std::string &bytes)
	{
		return Base64::encode((unsigned char*)bytes.data(), bytes.length());
	}
};

IMPLEMENT_TESTS(daedalusalpha);
