#include "adition.hpp"
#include <gtest/gtest.h>
#include "../test_base.hpp"


struct adition
{
	CBase64Parser p;
	std::string encode(std::string &bytes)
	{
		std::string out;
        uint32_t eLen=p.GetEncodeLen(bytes.length());
        out.resize(eLen);

        p.EncodeChunk((unsigned char*)bytes.data(), bytes.length(),&out[0]);
        return out;
	}

	std::string decode(std::string &bytes)
	{
        std::string out;
		uint32_t eLen=p.GetDecodeExpectedLen(bytes.length());
        out.resize(eLen);
		eLen=p.DecodeChunk(bytes.c_str(), bytes.length(),reinterpret_cast<uint8_t*>(&out[0]));
		out.resize(eLen);
        return out;
	}
};

IMPLEMENT_ENCODE_TESTS(adition);
IMPLEMENT_DECODE_TESTS(adition);
