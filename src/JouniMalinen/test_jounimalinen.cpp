#include "jounimalinen.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct jounimalinen
{
	std::string encode(std::string &bytes)
	{
		return base64_encode((unsigned char*)&bytes[0], bytes.length());
	}
};

IMPLEMENT_TESTS(jounimalinen);
