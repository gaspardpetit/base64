#include "jounimalinen.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct jounimalinen
{
	std::string encode(std::string &bytes)
	{
		return base64_encode((unsigned char*)&bytes[0], bytes.length());
	}

	std::string decode(std::string &bytes)
	{
		return base64_decode((unsigned char*)&bytes[0], bytes.length());
	}
};

IMPLEMENT_ENCODE_TESTS(jounimalinen);
IMPLEMENT_DECODE_TESTS(jounimalinen);
