#include "wikibooks_org_c.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"

struct WikibooksOrgC
{
	std::string encode(const std::string &bytes)
	{
		std::string out;
		size_t encLen = 4 * ((bytes.length() + 2) / 3);
		out.resize(encLen);
		base64encode(bytes.data(), bytes.length(), &out[0], encLen);
		return out;
	}

	std::string decode(const std::string &base64)
	{
		std::string out;
		size_t encLen = 3 * ((base64.length() + 3) / 4);
		out.resize(encLen);
		base64decode((char*)&base64[0], base64.length(), (unsigned char*)&out[0], &encLen);
		out.resize(encLen);
		return out;
	}
};

BASE64_REGISTER_ENCODER(WikibooksOrgC);
BASE64_REGISTER_DECODER(WikibooksOrgC);
