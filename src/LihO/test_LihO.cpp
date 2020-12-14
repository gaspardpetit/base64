#include "LihO.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"


struct LihO
{
	std::string encode(const std::string &bytes)
	{
		return LihO_base64_encode((unsigned char*)bytes.data(), bytes.length());
	}
	std::string decode(const std::string &base64)
	{
		return LihO_base64_decode(base64);
	}
};

BASE64_REGISTER_ENCODER(LihO);
BASE64_REGISTER_DECODER(LihO);
