#pragma once

#include <string>
::std::string base64_encode(const ::std::string &bindata);
::std::string base64_encode(const char *bytes, size_t length);
::std::string base64_decode(const ::std::string &ascdata);
