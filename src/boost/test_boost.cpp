#include "../test_base.hpp"
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>



using namespace boost::archive::iterators;

struct Boost
{
    std::string encode(std::string &binary)
    {
        using namespace boost::archive::iterators;
        using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
        auto base64 = std::string(It(binary.begin()), It(binary.end()));
        // Add padding.
        return base64.append((3 - binary.size() % 3) % 3, '=');
    }

	std::string decode(std::string &base64)
	{
        using namespace boost::archive::iterators;
        using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
        auto binary = std::string(It(base64.begin()), It(base64.end()));
        // Remove padding.
        auto length = base64.size();
        if (binary.size() > 2 && base64[length - 1] == '=' && base64[length - 2] == '=')
        {
            binary.erase(binary.end() - 2, binary.end());
        }
        else if (binary.size() > 1 && base64[length - 1] == '=')
        {
            binary.erase(binary.end() - 1, binary.end());
        }
        return binary;
    }
};

IMPLEMENT_ENCODE_TESTS(Boost);
IMPLEMENT_DECODE_TESTS(Boost);
