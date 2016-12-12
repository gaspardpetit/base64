#include "base64.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct adp_gmbh
{
	std::string encode(std::string &bytes)
	{
		return adp_gmbh_base64_encode((unsigned char*)bytes.data(), bytes.length());
	}
};

IMPLEMENT_TESTS(adp_gmbh);
