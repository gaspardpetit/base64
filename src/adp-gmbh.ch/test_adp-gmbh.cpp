#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"

namespace {
#include "ReneNyffenegger/cpp-base64/base64.cpp"
}

struct AdpGmbh
{
	std::string encode(const std::string &bytes)
	{
		return base64_encode((unsigned char*)bytes.data(), bytes.length());
	}

	std::string decode(const std::string &base64)
	{
		return base64_decode(base64);
	}
};

BASE64_REGISTER_ENCODER(AdpGmbh);
BASE64_REGISTER_DECODER(AdpGmbh);
