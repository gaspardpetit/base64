// workaround for https://github.com/boostorg/serialization/issues/315
#define BOOST_NO_EXCEPTIONS
namespace boost { template<class E> void throw_exception(E& e) {} }

#include "../Base64SurveyRegistry.hpp"
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
    std::string encode(const std::string &bytes)
    {
        using namespace boost::archive::iterators;
        using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
        auto base64 = std::string(It(bytes.begin()), It(bytes.end()));
        // Add padding.
        return base64.append((3 - bytes.size() % 3) % 3, '=');
    }

    std::string decode(const std::string &base64)
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

BASE64_REGISTER_ENCODER(Boost);
BASE64_REGISTER_DECODER(Boost);
