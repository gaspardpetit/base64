#include "../Base64SurveyRegistry.hpp"
#include <gtest/gtest.h>
#define BASE64_STATIC_DEFINE
#include "../libs/base64-aklomp/include/libbase64.h"

/**
* On Windows, to make sure to first build libs\base64-aklomp as a Visual Studio 2022 Cmake project to generate base64.lib under libs\base64-aklomp\out\build\x64-Release
**/

struct base64aklomp
{
	std::string encode(const std::string& bytes)
	{
		size_t outlen = (4*bytes.length() + 3 - 1) / 3;
                size_t margin = 10;
                int flags = 0; // the default implementation path
                //int flags = BASE64_FORCE_PLAIN; // the slower implementation path
                
                std::string outstr(outlen+margin, '\0');
                base64_encode(bytes.c_str(), bytes.length(), outstr.data(), &outlen, flags);
                outstr.resize(outlen);
                return outstr;
	}

	std::string decode(const std::string& base64)
	{
                size_t outlen = (3*base64.length() + 4 - 1) / 4;
                size_t margin = 10;
                int flags = 0; // the default implementation path
                //int flags = BASE64_FORCE_PLAIN; // the slower implementation path

                std::string outstr(outlen+margin, '\0');
                const int outval = base64_decode(base64.c_str(), base64.length(), outstr.data(), &outlen, flags);
                if(outval != 1)
                {
                        throw std::runtime_error("base64_decode went wrong with output value: " + std::to_string(outval));
                }
                outstr.resize(outlen);
                return outstr;
	}
};

BASE64_REGISTER_ENCODER(base64aklomp);
BASE64_REGISTER_DECODER(base64aklomp);
