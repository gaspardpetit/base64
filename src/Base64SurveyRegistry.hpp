#pragma once

#include "Base64SurveyTests.hpp"
#include <string>
#include <map>
#include <gtest/gtest.h>

namespace b64_survey {
using namespace std;

using Base64EncoderProc = string(*)(const string &);
using Base64DecoderProc = string(*)(const string &);

struct Base64SurveyRegistry
{
	static map<string, Base64EncoderProc>& GetEncoders();
	static map<string, Base64DecoderProc>& GetDecoders();

	template<class BASE64Impl>
	struct RegisterEncodeTest
	{
		explicit RegisterEncodeTest(const string &name)
		{
			GetEncoders()[name] = [](const string &bytes) {
				BASE64Impl impl;
				return impl.encode(bytes);
			};
		}
	};

	template<class BASE64Impl>
	struct RegisterDecodeTest
	{
		explicit RegisterDecodeTest(const string &name)
		{
			GetDecoders()[name] = [](const string &bytes) {
				BASE64Impl impl;
				return impl.decode(bytes);
			};
		}
	};
};

#define BASE64_REGISTER_ENCODER(TEST_TYPE)\
b64_survey::Base64SurveyRegistry::RegisterEncodeTest<TEST_TYPE> register_##TEST_TYPE##_encode(#TEST_TYPE);\
BASE64_REGISTER_ENCODER_TESTS(TEST_TYPE)

#define BASE64_REGISTER_DECODER(TEST_TYPE)\
b64_survey::Base64SurveyRegistry::RegisterDecodeTest<TEST_TYPE> register_##TEST_TYPE##_decode(#TEST_TYPE);\
BASE64_REGISTER_DECODER_TESTS(TEST_TYPE)

} // namespace b64_survey
