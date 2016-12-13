#include "arduino.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct arduino
{
	std::string encode(std::string &bytes)
	{
		std::string outStr;
		Base64::Encode(bytes.data(), bytes.length(), &outStr);
		return outStr;
	}
	std::string decode(std::string &bytes)
	{
		std::string outStr;
		Base64::Decode(bytes, &outStr);
		return outStr;
	}
};

IMPLEMENT_ENCODE_TESTS(arduino);
IMPLEMENT_DECODE_TESTS(arduino);

