#include "ElegantDice.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"

struct ElegantDice
{
	std::string encode(const std::string &bytes)
	{
		std::string str;
		base64_encode(str, (unsigned char*)bytes.data(), bytes.length());
		return str;
	}

	std::string decode(const std::string &base64)
	{
		std::string str;
		base64_decode(str, base64);
		return str;
	}
};


BASE64_REGISTER_ENCODER(ElegantDice);
BASE64_REGISTER_DECODER(ElegantDice);
