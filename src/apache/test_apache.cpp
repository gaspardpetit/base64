#include "apache.h"
#include <gtest/gtest.h>
#include "../Base64SurveyRegistry.hpp"

struct Apache
{
	std::string encode(const std::string &bytes)
	{
		std::string out;
		out.resize(Base64encode_len(bytes.length()));
		Base64encode(&out[0], bytes.data(), bytes.length());
		return out;
	}

	std::string decode(const std::string &base64)
	{
		std::string out;
		out.resize(Base64decode_len(&base64[0]));
		size_t actualSize = Base64decode(&out[0], base64.data());
		out.resize(actualSize);
		return out;
	}
};

BASE64_REGISTER_ENCODER(Apache);
BASE64_REGISTER_DECODER(Apache);
