#include "curl_base64.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"


struct LibCurl
{
	std::string encode(const std::string &bytes)
	{
		std::string str;
		Curl_base64_encode(bytes.data(), bytes.length(), &str);
		return str;
	}

	std::string decode(const std::string &base64)
	{
		std::string str;
		Curl_base64_decode(base64.data(), &str);
		return str;
	}
};

BASE64_REGISTER_ENCODER(LibCurl);
BASE64_REGISTER_DECODER(LibCurl);
