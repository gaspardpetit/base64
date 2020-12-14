#include "DaedalusAlpha.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"

struct DaedalusAlpha
{
	std::string encode(const std::string &bytes)
	{
		return Base64::encode((unsigned char*)bytes.data(), bytes.length());
	}
	std::string decode(const std::string &base64)
	{
		return Base64::decode(base64);
	}
};

BASE64_REGISTER_ENCODER(DaedalusAlpha);
BASE64_REGISTER_DECODER(DaedalusAlpha);
