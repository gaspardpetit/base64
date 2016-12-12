#ifndef _BASE64_H_
#define _BASE64_H_

#include <vector>
#include <string>

typedef unsigned char BYTE;

std::string LihO_base64_encode(BYTE const* buf, unsigned int bufLen);
std::vector<BYTE> LihO_base64_decode(std::string const&);

#endif
