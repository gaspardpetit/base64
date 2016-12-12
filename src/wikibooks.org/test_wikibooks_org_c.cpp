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
};

IMPLEMENT_TESTS(wikibooks_org_c);
