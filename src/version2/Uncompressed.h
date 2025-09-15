#pragma once

#include <cstdint>

struct UncompressedBoard
{
  uint8_t bkx;
  uint8_t bky;
  uint8_t wkx;
  uint8_t wky;
  uint8_t wrx;
  uint8_t wry;
};

struct UncompressedInfo
{
  uint16_t mate_in_ply_encoded:11;
  uint16_t classification:5;
  uint16_t number_of_children;
};

