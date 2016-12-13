#include "wikibooks_org_c.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct wikibooks_org_c
{
	std::string encode(std::string &bytes)
	{
		std::string out;
		size_t encLen = 4 * ((bytes.length() + 2) / 3);
		out.resize(encLen);
		base64encode(bytes.data(), bytes.length(), &out[0], encLen);
		return out;
	}

	std::string decode(std::string &bytes)
	{
		std::string out;
		size_t encLen = 3 * ((bytes.length() + 3) / 4);
		out.resize(encLen);
		base64decode(&bytes[0], bytes.length(), (unsigned char*)&out[0], &encLen);
		out.resize(encLen);
		return out;
	}
};

IMPLEMENT_ENCODE_TESTS(wikibooks_org_c);
IMPLEMENT_DECODE_TESTS(wikibooks_org_c);
