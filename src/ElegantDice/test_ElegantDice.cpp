#include "ElegantDice.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct elegantdice
{
	std::string encode(std::string &bytes)
	{
		std::string str;
		base64_encode(str, (unsigned char*)bytes.data(), bytes.length());
		return str;
	}
};


IMPLEMENT_TESTS(elegantdice);
