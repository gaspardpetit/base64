#include "tomykaria.hpp"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"


struct TomyKaria
{
	macaron::Base64 base;
    std::string encode(const std::string &bytes)
	{
		return base.Encode(bytes);
    }

	std::string decode(const std::string &base64)
	{
        std::string out;
        base.Decode(base64,out);
        return out;
	}
};

BASE64_REGISTER_ENCODER(TomyKaria);
BASE64_REGISTER_DECODER(TomyKaria);