#pragma once

#include <cstdint>
#include <utility>

// Return the smallest possible unsigned int that can contain `bits` bits.
template<int bits>
using uint_type =
  std::conditional_t<bits <=  8, uint8_t,
  std::conditional_t<bits <= 16, uint16_t,
  std::conditional_t<bits <= 32, uint32_t,
  std::conditional_t<bits <= 64, uint64_t,
      void>>>>;
