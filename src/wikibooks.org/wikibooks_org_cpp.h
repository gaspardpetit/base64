#include <string>
#include <vector>
#include <stdexcept>

// Prototype
// std::basic_string<TCHAR> base64Encode(std::vector<BYTE> inputBuffer);
// This line goes in header file

/* Define these if they aren't already in your environment
* #define TEXT(x) Lx    //Unicode
* #define TCHAR wchar_t //Unicode
* #define TCHAR char    //Not unicode
* #define TEXT(x) x     //Not unicode
* #define DWORD long
* #define BYTE unsigned char
* They are defined by default in Windows.h
*/

#define TEXT(x) Lx    //Unicode
//#define TCHAR wchar_t //Unicode
#define TCHAR char    //Not unicode
#define TEXT(x) x     //Not unicode
#define DWORD long
#define BYTE unsigned char

//Lookup table for encoding
//If you want to use an alternate alphabet, change the characters here
const static TCHAR encodeLookup[] = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
const static TCHAR padCharacter = TEXT('=');
std::basic_string<TCHAR> base64Encode(BYTE *inputBuffer, size_t length)
{
	std::basic_string<TCHAR> encodedString;
	encodedString.reserve(((length / 3) + (length % 3 > 0)) * 4);
	DWORD temp;
	BYTE *cursor = inputBuffer;
	for (size_t idx = 0; idx < length / 3; idx++)
	{
		temp = (*cursor++) << 16; //Convert to big endian
		temp += (*cursor++) << 8;
		temp += (*cursor++);
		encodedString.append(1, encodeLookup[(temp & 0x00FC0000) >> 18]);
		encodedString.append(1, encodeLookup[(temp & 0x0003F000) >> 12]);
		encodedString.append(1, encodeLookup[(temp & 0x00000FC0) >> 6]);
		encodedString.append(1, encodeLookup[(temp & 0x0000003F)]);
	}
	switch (length % 3)
	{
	case 1:
		temp = (*cursor++) << 16; //Convert to big endian
		encodedString.append(1, encodeLookup[(temp & 0x00FC0000) >> 18]);
		encodedString.append(1, encodeLookup[(temp & 0x0003F000) >> 12]);
		encodedString.append(2, padCharacter);
		break;
	case 2:
		temp = (*cursor++) << 16; //Convert to big endian
		temp += (*cursor++) << 8;
		encodedString.append(1, encodeLookup[(temp & 0x00FC0000) >> 18]);
		encodedString.append(1, encodeLookup[(temp & 0x0003F000) >> 12]);
		encodedString.append(1, encodeLookup[(temp & 0x00000FC0) >> 6]);
		encodedString.append(1, padCharacter);
		break;
	}
	return encodedString;
}


#include <string>
#include <vector>

// Prototype
// std::vector<BYTE> base64Decode(const std::basic_string<TCHAR>& input);
// This line goes in header file

/* Define these if they aren't already in your environment
* #define TEXT(x) Lx    //Unicode
* #define TCHAR wchar_t //Unicode
* #define TCHAR char    //Not unicode
* #define TEXT(x) x     //Not unicode
* #define DWORD long
* #define BYTE unsigned char
* They are defined by default in Windows.h
*/

std::string base64Decode(const std::basic_string<TCHAR>& input)
{
	if (input.length() % 4) //Sanity check
		throw std::runtime_error("Non-Valid base64!");
	size_t padding = 0;
	if (input.length())
	{
		if (input[input.length() - 1] == padCharacter)
			padding++;
		if (input[input.length() - 2] == padCharacter)
			padding++;
	}
	//Setup a vector to hold the result
	std::string decodedBytes;
	decodedBytes.reserve(((input.length() / 4) * 3) - padding);
	DWORD temp = 0; //Holds decoded quanta
	std::basic_string<TCHAR>::const_iterator cursor = input.begin();
	while (cursor < input.end())
	{
		for (size_t quantumPosition = 0; quantumPosition < 4; quantumPosition++)
		{
			temp <<= 6;
			if (*cursor >= 0x41 && *cursor <= 0x5A) // This area will need tweaking if
				temp |= *cursor - 0x41;		              // you are using an alternate alphabet
			else if (*cursor >= 0x61 && *cursor <= 0x7A)
				temp |= *cursor - 0x47;
			else if (*cursor >= 0x30 && *cursor <= 0x39)
				temp |= *cursor + 0x04;
			else if (*cursor == 0x2B)
				temp |= 0x3E; //change to 0x2D for URL alphabet
			else if (*cursor == 0x2F)
				temp |= 0x3F; //change to 0x5F for URL alphabet
			else if (*cursor == padCharacter) //pad
			{
				switch (input.end() - cursor)
				{
				case 1: //One pad character
					decodedBytes.push_back((temp >> 16) & 0x000000FF);
					decodedBytes.push_back((temp >> 8) & 0x000000FF);
					return decodedBytes;
				case 2: //Two pad characters
					decodedBytes.push_back((temp >> 10) & 0x000000FF);
					return decodedBytes;
				default:
					throw std::runtime_error("Invalid Padding in Base 64!");
				}
			}
			else
				throw std::runtime_error("Non-Valid Character in Base 64!");
			cursor++;
		}
		decodedBytes.push_back((temp >> 16) & 0x000000FF);
		decodedBytes.push_back((temp >> 8) & 0x000000FF);
		decodedBytes.push_back((temp) & 0x000000FF);
	}
	return decodedBytes;
}
