#include "ahristov.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"

struct Ahristov
{
	std::string encode(const std::string &bytes)
	{
		return encodeBase64(bytes);
	}

	std::string decode(const std::string &base64)
	{
		return decodeBase64(base64);
	}
};

BASE64_REGISTER_ENCODER(Ahristov);
BASE64_REGISTER_DECODER(Ahristov);