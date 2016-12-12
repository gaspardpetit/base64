#include "polfosol.h"
#include <gtest/gtest.h>
#include "../test_base.hpp"

struct polfosol
{
	std::string encode(std::string &bytes)
	{
		return b64encode((unsigned char*)bytes.data(), bytes.length());
	}
};

IMPLEMENT_TESTS(polfosol);
