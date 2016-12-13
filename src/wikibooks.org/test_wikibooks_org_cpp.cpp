#include "wikibooks_org_cpp.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct wikibooks_org_cpp
{
	std::string encode(std::string &bytes)
	{
		std::string out;
		size_t encLen = 4 * ((bytes.length() + 2) / 3);
		out.resize(encLen);
		return base64Encode((unsigned char*)bytes.data(), bytes.length());
	}

	std::string decode(std::string &bytes)
	{
		std::string out;
		size_t encLen = 4 * ((bytes.length() + 2) / 3);
		out.resize(encLen);
		return base64Decode(bytes);
	}
};

IMPLEMENT_ENCODE_TESTS(wikibooks_org_cpp);
IMPLEMENT_DECODE_TESTS(wikibooks_org_cpp);
