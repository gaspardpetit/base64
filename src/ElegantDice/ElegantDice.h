#pragma once

#include <vector>
#include <string>
#include <cstdint>

void base64_encode(std::string & out, const std::vector<std::uint8_t>& buf);
void base64_encode(std::string & out, const std::uint8_t* buf, size_t bufLen);
void base64_encode(std::string & out, std::string const& buf);

void base64_decode(std::vector<std::uint8_t> & out, std::string const& encoded_string);

// use this if you know the output should be a valid string
void base64_decode(std::string & out, std::string const& encoded_string);

