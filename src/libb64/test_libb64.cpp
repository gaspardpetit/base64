#define BUFFERSIZE 65536
#include "libb64/include/b64/encode.h"
#include "libb64/include/b64/decode.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct libb64
{
	static base64::encoder e;
	static base64::decoder d;

	std::string encode(std::string &str)
	{
		std::istringstream istrm(str);
		std::ostringstream ostrm;
		e.encode(istrm, ostrm);
		return std::move(ostrm.str());
	}

	std::string decode(std::string &str)
	{
		std::istringstream istrm(str);
		std::ostringstream ostrm;
		d.decode(istrm, ostrm);
		return std::move(ostrm.str());
	}
};

base64::encoder libb64::e;
base64::decoder libb64::d;

IMPLEMENT_ENCODE_TESTS(libb64);
IMPLEMENT_DECODE_TESTS(libb64);
