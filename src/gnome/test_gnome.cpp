#include "gnome.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"


struct Gnome
{
	std::string encode(const std::string &bytes)
	{
		return g_base64_encode((unsigned char*)bytes.data(), bytes.length());
	}

	std::string decode(const std::string &base64)
	{
		return g_base64_decode(base64.data());
	}
};

BASE64_REGISTER_ENCODER(Gnome);
BASE64_REGISTER_DECODER(Gnome);
