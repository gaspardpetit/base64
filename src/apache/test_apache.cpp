#include "apache.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct apache
{
	std::string encode(std::string &bytes)
	{
		std::string out;
		out.resize(Base64encode_len(bytes.length()));
		Base64encode(&out[0], bytes.data(), bytes.length());
		return out;
	}
};

IMPLEMENT_TESTS(apache);
