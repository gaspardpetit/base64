#pragma once
#include <string>

std::map<std::string, std::string(*)(std::string &)>& getTests();

extern std::string s_tiny;
extern std::string s_tiny_expected;
extern std::string s_short;
extern std::string s_short_expected;
extern std::string s_medium;
extern std::string s_medium_expected;
extern std::string s_long;
extern std::string s_long_expected;


template<class BASE64Impl>
struct RegisterTest
{
	RegisterTest(const std::string name)
	{
		getTests()[name] = [](std::string &bytes) {
			BASE64Impl impl;
			return impl.encode(bytes);
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


#define IMPLEMENT_TESTS(TEST_TYPE)\
RegisterTest<TEST_TYPE> register_##TEST_TYPE(#TEST_TYPE);\
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
