#pragma once

#include <cstdint>
#include <iostream>
#include <vector>

/**
 * @brief Base64 encoder and decoder class
 *
 */

class CBase64Parser final {
private:
  
public:
  /**
   * @brief 
   * 
   * @param in 
   * @param inLen 
   * @param out 
   */
  void EncodeChunk(const uint8_t *in, uint32_t inLen, char *out) const ;

  /**
   * @brief 
   * 
   * @param in 
   * @param inLen 
   * @param out 
   * @return uint32_t 
   */
  uint32_t DecodeChunk(const char *in, uint32_t inLen,uint8_t *out) const ;
  
  /**
   * @brief Get the Encode Len object
   * 
   * @param inLen 
   * @return uint32_t 
   */
  uint32_t GetEncodeLen(uint32_t inLen) const;

  /**
   * @brief Get the Decode Expected Len object
   * 
   * @param inLen 
   * @return uint32_t 
   */
  uint32_t GetDecodeExpectedLen(uint32_t inLen) const;

  /**
   * @brief 
   * 
   * @param in 
   * @return std::vector<uint8_t> 
   */
  std::vector<uint8_t> Decode(const std::string &in) const;

  /**
   * @brief 
   * 
   * @param in 
   * @return std::string 
   */
  std::string Encode(const std::vector<uint8_t>& in) const;
};