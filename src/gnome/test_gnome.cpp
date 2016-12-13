#include "gnome.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct gnome
{
	std::string encode(std::string &bytes)
	{
		return g_base64_encode((unsigned char*)bytes.data(), bytes.length());
	}

	std::string decode(std::string &bytes)
	{
		return g_base64_decode(bytes.data());
	}
};

IMPLEMENT_ENCODE_TESTS(gnome);
IMPLEMENT_DECODE_TESTS(gnome);
