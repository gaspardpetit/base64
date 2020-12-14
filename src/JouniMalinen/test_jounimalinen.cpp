#include "jounimalinen.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"

struct JouniMalinen
{
	std::string encode(const std::string &bytes)
	{
		return base64_encode((unsigned char*)&bytes[0], bytes.length());
	}

	std::string decode(const std::string &base64)
	{
		return base64_decode((unsigned char*)&base64[0], base64.length());
	}
};

BASE64_REGISTER_ENCODER(JouniMalinen);
BASE64_REGISTER_DECODER(JouniMalinen);
