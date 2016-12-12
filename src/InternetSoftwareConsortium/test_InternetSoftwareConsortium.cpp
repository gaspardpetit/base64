#include "InternetSoftwareConsortium.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct internetsoftwareconsortium
{
	std::string encode(std::string &bytes)
	{
		std::string outStr;
		size_t outLength = 4 * ((bytes.length() + 2) / 3);
		outStr.resize(outLength+1);
		size_t realLength = b64_ntop((unsigned char*)bytes.data(), bytes.length(), &outStr[0], outLength + 1);
		outStr.resize(realLength);
		return outStr;
	}
};

IMPLEMENT_TESTS(internetsoftwareconsortium);
