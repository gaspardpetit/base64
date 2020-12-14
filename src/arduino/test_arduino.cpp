#include "arduino.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"


struct Arduino
{
	std::string encode(const std::string &bytes)
	{
		std::string outStr;
		Base64::Encode(bytes.data(), bytes.length(), &outStr);
		return outStr;
	}
	std::string decode(const std::string &base64)
	{
		std::string outStr;
		Base64::Decode(base64, &outStr);
		return outStr;
	}
};

BASE64_REGISTER_ENCODER(Arduino);
BASE64_REGISTER_DECODER(Arduino);

