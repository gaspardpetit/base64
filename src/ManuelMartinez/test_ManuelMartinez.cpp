#include "manuelmartinez.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct manuelmartinez
{
	std::string encode(std::string &bytes)
	{
		return base64_encode((unsigned char*)bytes.data(), bytes.length());
	}
};

IMPLEMENT_TESTS(manuelmartinez);
