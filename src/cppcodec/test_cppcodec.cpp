#include "cppcodec/base64_default_rfc4648.hpp"
#include <iostream>
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"


struct CppCodec
{
	std::string encode(const std::string &bytes)
	{
		return base64::encode((unsigned char*)bytes.data(), bytes.length());
	}

	std::string decode(const std::string &base64)
	{
		return base64::decode<std::string>((char*)base64.data(), base64.length());
	}
};

BASE64_REGISTER_ENCODER(CppCodec);
BASE64_REGISTER_DECODER(CppCodec);
