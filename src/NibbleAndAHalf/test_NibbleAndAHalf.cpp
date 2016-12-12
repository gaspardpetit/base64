#include "NibbleAndAHalf.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct NibbleAndAHalf
{
	std::string encode(std::string &bytes)
	{
		return base64(bytes.data(), bytes.length());
	}
};

IMPLEMENT_TESTS(NibbleAndAHalf);
