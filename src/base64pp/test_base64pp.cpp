#include "../Base64SurveyRegistry.hpp"
#include <gtest/gtest.h>
#include "../libs/base64pp/base64pp/include/base64pp/base64pp.h"


struct base64ppwrap
{
	std::string encode(const std::string& bytes)
	{
                return base64pp::encode_str(bytes);
	}

	std::string decode(const std::string& base64)
	{
                auto const decoded_bytes = base64pp::decode(base64);
                if (!decoded_bytes) throw std::runtime_error{"Invalid base64"};
                return std::string(decoded_bytes->begin(), decoded_bytes->end());
	}
};

BASE64_REGISTER_ENCODER(base64ppwrap);
BASE64_REGISTER_DECODER(base64ppwrap);
