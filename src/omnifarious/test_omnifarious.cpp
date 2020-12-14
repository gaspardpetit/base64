#include "omnifarious.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"


struct Omnifarious
{
	std::string encode(const std::string &bytes)
	{
		return base64_encode(bytes.data(), bytes.length());
	}
};

BASE64_REGISTER_ENCODER(Omnifarious);

