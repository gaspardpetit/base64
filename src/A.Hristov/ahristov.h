#include <string>
#include <cstdint>
/**
 * Given a string, this function will encode it in 64b (with padding)
 */
std::string encodeBase64(const std::string& binaryText);

/**
 * Given a 64b padding-encoded string, this function will decode it.
 */
std::string decodeBase64(const std::string& base64Text);
