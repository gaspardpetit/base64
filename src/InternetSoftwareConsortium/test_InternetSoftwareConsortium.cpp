#include "InternetSoftwareConsortium.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"


struct InternetSoftwareConsortium
{
	std::string encode(const std::string &bytes)
	{
		std::string outStr;
		size_t outLength = 4 * ((bytes.length() + 2) / 3);
		outStr.resize(outLength + 1);
		size_t realLength = b64_ntop((unsigned char*)bytes.data(), bytes.length(), &outStr[0], outLength + 1);
		outStr.resize(realLength);
		return outStr;
	}

	std::string decode(const std::string &base64)
	{
		std::string outStr;
		size_t outLength = 3 * ((base64.length() + 3) / 4);
		outStr.resize(outLength + 1);
		size_t realLength = b64_pton(base64.data(), (unsigned char*)&outStr[0], outLength + 1);
		outStr.resize(realLength);
		return outStr;
	}
};

BASE64_REGISTER_ENCODER(InternetSoftwareConsortium);
BASE64_REGISTER_DECODER(InternetSoftwareConsortium);
