#pragma once

#include <vector>
#include <string>
typedef unsigned char BYTE;

class Base64
{
public:
	static std::string encode(const std::vector<BYTE>& buf);
	static std::string encode(const BYTE* buf, unsigned int bufLen);
	static std::string decode(std::string encoded_string);
};