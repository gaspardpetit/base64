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

	std::string decode(std::string &bytes)
	{
		std::string str;
		Curl_base64_decode(bytes.data(), &str);
		return str;
	}
};

IMPLEMENT_ENCODE_TESTS(libcurl);
IMPLEMENT_DECODE_TESTS(libcurl);
