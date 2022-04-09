#include "polfosol_imutility.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"

struct Polfosol_IMUtility
{
	std::string encode(const std::string &bytes)
	{
		return polfosol_imutility::Base64_encode((unsigned char*)bytes.data(), bytes.length());
	}

	std::string decode(const std::string &base64)
	{
		return polfosol_imutility::Base64_decode((unsigned char*)base64.data(), base64.length());
	}
};


BASE64_REGISTER_ENCODER(Polfosol_IMUtility);
BASE64_REGISTER_DECODER(Polfosol_IMUtility);
