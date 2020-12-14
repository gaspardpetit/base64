#include "user152949.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"

struct User152949
{
	std::string encode(const std::string &bytes)
	{
		std::istringstream istrm(bytes);
		std::ostringstream ostrm;
		CBase64::Encode(istrm, ostrm);
		return std::move(ostrm.str());
	}

	std::string decode(const std::string &base64)
	{
		std::istringstream istrm(base64);
		std::ostringstream ostrm;
		CBase64::Decode(istrm, ostrm);
		return std::move(ostrm.str());
	}
};

BASE64_REGISTER_ENCODER(User152949);
BASE64_REGISTER_DECODER(User152949);
