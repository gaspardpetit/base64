#include <string>
#include <vector>

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
#define TCHAR wchar_t //Unicode
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
