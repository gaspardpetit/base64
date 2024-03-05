#include "../Base64SurveyRegistry.hpp"
#include <gtest/gtest.h>
#include "../libs/base64-tl/include/base64.hpp"


struct base64tl
{
	std::string encode(const std::string& bytes)
	{
		return base64::to_base64(bytes);
	}

	std::string decode(const std::string& base64)
	{
		return base64::from_base64(base64);
	}
};

BASE64_REGISTER_ENCODER(base64tl);
BASE64_REGISTER_DECODER(base64tl);
