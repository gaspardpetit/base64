#include "renenyffenegger.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct renenyffenegger_boost
{
	std::string encode(std::string &bytes)
	{
		return base64_encode(bytes);
    }

	std::string decode(std::string &bytes)
	{
        return base64_decode(bytes);
	}
};

IMPLEMENT_ENCODE_TESTS(renenyffenegger_boost);
IMPLEMENT_DECODE_TESTS(renenyffenegger_boost);