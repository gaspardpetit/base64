#define BUFFERSIZE 65536
#include "b64/encode.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct libb64
{
	static base64::encoder e;

	std::string encode(std::string &str)
	{
		std::istringstream istrm(str);
		std::ostringstream ostrm;
		e.encode(istrm, ostrm);
		return std::move(ostrm.str());
	}
};

base64::encoder libb64::e;

IMPLEMENT_TESTS(libb64);
