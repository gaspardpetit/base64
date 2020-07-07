#include "tomykaria.hpp"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct tomykaria
{
	macaron::Base64 base;
    std::string encode(std::string &bytes)
	{
		return base.Encode(bytes);
    }

	std::string decode(std::string &bytes)
	{
        std::string out;
        base.Decode(bytes,out);
        return out;
	}
};

IMPLEMENT_ENCODE_TESTS(tomykaria);
IMPLEMENT_DECODE_TESTS(tomykaria);