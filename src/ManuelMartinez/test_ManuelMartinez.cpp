#include "ManuelMartinez.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct manuelmartinez
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

IMPLEMENT_ENCODE_TESTS(manuelmartinez);
IMPLEMENT_DECODE_TESTS(manuelmartinez);
