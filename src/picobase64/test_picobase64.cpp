#include "../Base64SurveyRegistry.hpp"
#include <gtest/gtest.h>
#include "../libs/picobase64/picobase64.h"


struct picobase64
{
	std::string encode(const std::string& bytes)
	{
		return b64encode(bytes);
	}

	std::string decode(const std::string& base64)
	{
		return b64decode(base64);
	}
};

BASE64_REGISTER_ENCODER(picobase64);
BASE64_REGISTER_DECODER(picobase64);
