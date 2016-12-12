#include "user152949.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct user152949
{
	std::string encode(std::string &bytes)
	{
		std::istringstream istrm(bytes);
		std::ostringstream ostrm;
		CBase64::Encode(istrm, ostrm);
		return std::move(ostrm.str());
	}
};

IMPLEMENT_TESTS(user152949);
