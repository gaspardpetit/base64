#include "ahristov.h"
#include <cassert>
#include <array>

const char PADDING_CHAR = '=';

/**
 * Given a string, this function will encode it in 64b (with padding)
 */
std::string encodeBase64(const std::string& binaryText)
{
    constexpr char ALPHABET[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string encoded((binaryText.size()/3 + (binaryText.size()%3 > 0)) << 2, PADDING_CHAR);

    const uint8_t *bytes = reinterpret_cast<const uint8_t*>(&binaryText[0]);
    char* currEncoding = &encoded[0];

    union
    {
        uint32_t temp = 0;
        struct
        {
            uint32_t first : 6, second : 6, third : 6, fourth : 6;
        } tempBytes;
    };
    
    for(uint32_t i = binaryText.size()/3; i; --i, bytes+=3, currEncoding+=4)
    {
        temp = bytes[0] << 16 | bytes[1] << 8 | bytes[2];

        currEncoding[0] = ALPHABET[tempBytes.fourth];
        currEncoding[1] = ALPHABET[tempBytes.third];
        currEncoding[2] = ALPHABET[tempBytes.second];
        currEncoding[3] = ALPHABET[tempBytes.first];
    }

    switch(binaryText.size() % 3)
    {
    case 1:
        temp = bytes[0] << 16;

        currEncoding[0] = ALPHABET[tempBytes.fourth];
        currEncoding[1] = ALPHABET[tempBytes.third];
        break;
    case 2:
        temp = bytes[0] << 16 | bytes[1] << 8;

        currEncoding[0] = ALPHABET[tempBytes.fourth];
        currEncoding[1] = ALPHABET[tempBytes.third];
        currEncoding[2] = ALPHABET[tempBytes.second];
        break;
    }

    return encoded;
}

/**
 * Given a 64b padding-encoded string, this function will decode it.
 */
std::string decodeBase64(const std::string& base64Text)
{
    constexpr uint8_t DECODED_ALPHBET[128]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,0,0,0,63,52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,0,0,0,0,0};

    if( base64Text.empty() )
        return "";

    assert((base64Text.size()&3) == 0 && "The base64 text to be decoded must have a length devisible by 4!");

    uint32_t numPadding = (*std::prev(base64Text.end(),1) == PADDING_CHAR) + (*std::prev(base64Text.end(),2) == PADDING_CHAR);

    std::string decoded((base64Text.size()*3>>2) - numPadding, '.');

    union
    {
        uint32_t temp;
        uint8_t tempBytes[4];
    };
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&base64Text[0]);
    char* currDecoding = &decoded[0];

    for(uint32_t i = (base64Text.size() >> 2) - (numPadding!=0); i; --i, bytes+=4, currDecoding+=3)
    {
        temp = DECODED_ALPHBET[bytes[0]] << 18 | DECODED_ALPHBET[bytes[1]] << 12 | DECODED_ALPHBET[bytes[2]] << 6 | DECODED_ALPHBET[bytes[3]];
        currDecoding[0] = tempBytes[2];
        currDecoding[1] = tempBytes[1];
        currDecoding[2] = tempBytes[0];
    }

    switch (numPadding)
    {
    case 2:
        temp = DECODED_ALPHBET[bytes[0]] << 18 | DECODED_ALPHBET[bytes[1]] << 12;
        currDecoding[0] = tempBytes[2];
        break;
    
    case 1:
        temp = DECODED_ALPHBET[bytes[0]] << 18 | DECODED_ALPHBET[bytes[1]] << 12 | DECODED_ALPHBET[bytes[2]] << 6;
        currDecoding[0] = tempBytes[2];
        currDecoding[1] = tempBytes[1];
        break;
    }

    return decoded;
}