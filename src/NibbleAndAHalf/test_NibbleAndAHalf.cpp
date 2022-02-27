#include "NibbleAndAHalf.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"

struct NibbleAndAHalf
{
	std::string encode(const std::string &bytes)
	{
		return base64(bytes.data(), bytes.length());
	}

	std::string decode(const std::string &base64)
	{
		return unbase64(base64.data(), base64.length());
	}
};

BASE64_REGISTER_ENCODER(NibbleAndAHalf);
BASE64_REGISTER_DECODER(NibbleAndAHalf);
