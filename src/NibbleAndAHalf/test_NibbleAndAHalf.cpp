#include "NibbleAndAHalf.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct NibbleAndAHalf
{
	std::string encode(std::string &bytes)
	{
		return base64(bytes.data(), bytes.length());
	}

	std::string decode(std::string &bytes)
	{
		return unbase64(bytes.data(), bytes.length());
	}
};

IMPLEMENT_ENCODE_TESTS(NibbleAndAHalf);
