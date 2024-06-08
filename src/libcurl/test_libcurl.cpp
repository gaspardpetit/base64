#include "curl_base64.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"


struct LibCurl
{
	std::string encode(const std::string &bytes)
	{
		char* out;
		size_t outlen;
		Curl_base64_encode(bytes.data(), bytes.length(), &out, &outlen);
		std::string str(out, outlen);
		free(out);
		return str;
	}

	std::string decode(const std::string &base64)
	{
		unsigned char* out;
		size_t outlen;
		Curl_base64_decode(base64.data(), &out, &outlen);
		std::string str((char*)out, outlen);
		free(out);
		return str;
	}
};

BASE64_REGISTER_ENCODER(LibCurl);
BASE64_REGISTER_DECODER(LibCurl);
