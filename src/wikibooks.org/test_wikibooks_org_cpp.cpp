#include "wikibooks_org_cpp.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"

struct WikibooksOrgCpp
{
	std::string encode(const std::string &bytes)
	{
		std::string out;
		size_t encLen = 4 * ((bytes.length() + 2) / 3);
		out.resize(encLen);
		return base64Encode((unsigned char*)bytes.data(), bytes.length());
	}

	std::string decode(const std::string &base64)
	{
		std::string out;
		size_t encLen = 4 * ((base64.length() + 2) / 3);
		out.resize(encLen);
		return base64Decode(base64);
	}
};

BASE64_REGISTER_ENCODER(WikibooksOrgCpp);
BASE64_REGISTER_DECODER(WikibooksOrgCpp);
