#pragma once

#include "Base64SurveyRegistry.hpp"
#include <gtest/gtest.h>
#include <string>

namespace b64_survey {
using namespace std;

struct Base64SurveyTests
{
	static const string s_tiny;
	static const string s_tiny_expected;
	static const string s_short;
	static const string s_short_expected;
	static const string s_medium;
	static const string s_medium_expected;
	static const string s_long;
	static const string s_long_expected;

	template<class BASE64Impl>
	static void test_encode(const string &str, const string &expected)
	{
		BASE64Impl impl;
		string observed = impl.encode(str);
		ASSERT_EQ(expected, observed);
	}

	template<class BASE64Impl>
	static void test_decode(const string &str, const string &expected)
	{
		BASE64Impl impl;
		string observed = impl.decode(str);
		ASSERT_EQ(expected, observed);
	}

	static int RunTests(int argc, char *argv[]);
};


#define BASE64_REGISTER_ENCODER_TESTS(TEST_TYPE)\
TEST(TEST_TYPE, test_encode_tiny)\
{\
	b64_survey::Base64SurveyTests::test_encode<TEST_TYPE>(b64_survey::Base64SurveyTests::s_tiny, b64_survey::Base64SurveyTests::s_tiny_expected);\
}\
\
TEST(TEST_TYPE, test_encode_short)\
{\
	b64_survey::Base64SurveyTests::test_encode<TEST_TYPE>(b64_survey::Base64SurveyTests::s_short, b64_survey::Base64SurveyTests::s_short_expected);\
}\
\
TEST(TEST_TYPE, test_encode_medium)\
{\
	b64_survey::Base64SurveyTests::test_encode<TEST_TYPE>(b64_survey::Base64SurveyTests::s_medium, b64_survey::Base64SurveyTests::s_medium_expected);\
}\
\
TEST(TEST_TYPE, test_encode_long)\
{\
	b64_survey::Base64SurveyTests::test_encode<TEST_TYPE>(b64_survey::Base64SurveyTests::s_long, b64_survey::Base64SurveyTests::s_long_expected);\
}

#define BASE64_REGISTER_DECODER_TESTS(TEST_TYPE)\
TEST(TEST_TYPE, test_decode_tiny)\
{\
	b64_survey::Base64SurveyTests::test_decode<TEST_TYPE>(b64_survey::Base64SurveyTests::s_tiny_expected, b64_survey::Base64SurveyTests::s_tiny);\
}\
\
TEST(TEST_TYPE, test_decode_short)\
{\
	b64_survey::Base64SurveyTests::test_decode<TEST_TYPE>(b64_survey::Base64SurveyTests::s_short_expected, b64_survey::Base64SurveyTests::s_short);\
}\
\
TEST(TEST_TYPE, test_decode_medium)\
{\
	b64_survey::Base64SurveyTests::test_decode<TEST_TYPE>(b64_survey::Base64SurveyTests::s_medium_expected, b64_survey::Base64SurveyTests::s_medium);\
}\
\
TEST(TEST_TYPE, test_decode_long)\
{\
	b64_survey::Base64SurveyTests::test_decode<TEST_TYPE>(b64_survey::Base64SurveyTests::s_long_expected, b64_survey::Base64SurveyTests::s_long);\
}

} // namespace b64_survey
