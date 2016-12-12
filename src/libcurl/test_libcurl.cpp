#include "curl_base64.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct libcurl
{
	std::string encode(std::string &bytes)
	{
		std::string str;
		Curl_base64_encode(bytes.data(), bytes.length(), &str);
		return str;
	}
};

IMPLEMENT_TESTS(libcurl);
