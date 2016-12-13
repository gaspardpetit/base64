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

	std::string decode(std::string &bytes)
	{
		std::string out;
		out.resize(Base64decode_len(&bytes[0]));
		size_t actualSize = Base64decode(&out[0], bytes.data());
		out.resize(actualSize);
		return out;
	}
};

IMPLEMENT_ENCODE_TESTS(apache);
IMPLEMENT_DECODE_TESTS(apache);
