#pragma once
#include <string>
#include <map>
#include <gtest/gtest.h>

std::map<std::string, std::string(*)(std::string &)>& getEncodeTests();
std::map<std::string, std::string(*)(std::string &)>& getDecodeTests();

extern std::string s_tiny;
extern std::string s_tiny_expected;
extern std::string s_short;
extern std::string s_short_expected;
extern std::string s_medium;
extern std::string s_medium_expected;
extern std::string s_long;
extern std::string s_long_expected;


template<class BASE64Impl>
struct RegisterEncodeTest
{
	RegisterEncodeTest(const std::string name)
	{
		getEncodeTests()[name] = [](std::string &bytes) {
			BASE64Impl impl;
			return impl.encode(bytes);
		};
	}
};

template<class BASE64Impl>
struct RegisterDecodeTest
{
	RegisterDecodeTest(const std::string name)
	{

		getDecodeTests()[name] = [](std::string &bytes) {
			BASE64Impl impl;
			return impl.decode(bytes);
		};
	}
};

template<class BASE64Impl>
void test_encode_tiny()
{
	std::string &str = s_tiny;
	std::string &expected = s_tiny_expected;

	BASE64Impl impl;
	ASSERT_EQ(expected, impl.encode(str));
}

template<class BASE64Impl>
void test_encode_short()
{
	std::string &str = s_short;
	std::string &expected = s_short_expected;

	BASE64Impl impl;
	ASSERT_EQ(expected, impl.encode(str));
}

template<class BASE64Impl>
void test_encode_medium()
{
	std::string &str = s_medium;
	std::string &expected = s_medium_expected;

	BASE64Impl impl;
	ASSERT_EQ(expected, impl.encode(str));
}

template<class BASE64Impl>
void test_encode_long()
{
	std::string &str = s_long;
	std::string &expected = s_long_expected;

	BASE64Impl impl;
	ASSERT_EQ(expected, impl.encode(str));
}


template<class BASE64Impl>
void test_decode_tiny()
{
	std::string &str = s_tiny_expected;
	std::string &expected = s_tiny;

	BASE64Impl impl;
	ASSERT_EQ(expected, impl.decode(str));
}

template<class BASE64Impl>
void test_decode_short()
{
	std::string &str = s_short_expected;
	std::string &expected = s_short;

	BASE64Impl impl;
	ASSERT_EQ(expected, impl.decode(str));
}

template<class BASE64Impl>
void test_decode_medium()
{
	std::string &str = s_medium_expected;
	std::string &expected = s_medium;

	BASE64Impl impl;
	ASSERT_EQ(expected, impl.decode(str));
}

template<class BASE64Impl>
void test_decode_long()
{
	std::string &str = s_long_expected;
	std::string &expected = s_long;

	BASE64Impl impl;
	ASSERT_EQ(expected, impl.decode(str));
}

#define IMPLEMENT_ENCODE_TESTS(TEST_TYPE)\
RegisterEncodeTest<TEST_TYPE> register_##TEST_TYPE##_encode(#TEST_TYPE);\
TEST(TEST_TYPE, test_encode_tiny)\
{\
	test_encode_tiny<TEST_TYPE>();\
}\
\
TEST(TEST_TYPE, test_encode_short)\
{\
	test_encode_short<TEST_TYPE>();\
}\
\
TEST(TEST_TYPE, test_encode_medium)\
{\
	test_encode_medium<TEST_TYPE>();\
}\
\
TEST(TEST_TYPE, test_encode_long)\
{\
	test_encode_long<TEST_TYPE>();\
}

#define IMPLEMENT_DECODE_TESTS(TEST_TYPE)\
RegisterDecodeTest<TEST_TYPE> register_##TEST_TYPE##_decode(#TEST_TYPE);\
TEST(TEST_TYPE, test_decode_tiny)\
{\
	test_decode_tiny<TEST_TYPE>();\
}\
\
TEST(TEST_TYPE, test_decode_short)\
{\
	test_decode_short<TEST_TYPE>();\
}\
\
TEST(TEST_TYPE, test_decode_medium)\
{\
	test_decode_medium<TEST_TYPE>();\
}\
\
TEST(TEST_TYPE, test_decode_long)\
{\
	test_decode_long<TEST_TYPE>();\
}
