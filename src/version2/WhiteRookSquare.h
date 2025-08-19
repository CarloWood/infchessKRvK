#pragma once

#include "SquareCompact.h"

// This class exist of an unsigned integer that uses the Size::board::square_bits least significant bits.
// The other bits are guaranteed to be zero.
class WhiteRookSquare : public SquareCompact<Size::board>
{
 public:
  static constexpr int bits              = Size::board::square_bits;         // Required bits.
  static constexpr coordinates_type mask = Size::board::square_mask;

  WhiteRookSquare(coordinates_type square) : SquareCompact<Size::board>(square) { }
  WhiteRookSquare(int x, int y) : SquareCompact<Size::board>(x, y) { }
};
