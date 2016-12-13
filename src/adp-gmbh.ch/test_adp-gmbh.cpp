#include "base64.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct adp_gmbh
{
	std::string encode(std::string &bytes)
	{
		return adp_gmbh_base64_encode((unsigned char*)bytes.data(), bytes.length());
	}

	std::string decode(std::string &bytes)
	{
		return adp_gmbh_base64_decode(bytes);
	}
};

IMPLEMENT_ENCODE_TESTS(adp_gmbh);
IMPLEMENT_DECODE_TESTS(adp_gmbh);
