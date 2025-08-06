#pragma once

#include "Size.h"
#include "SquareCompact.h"

class BlockSquareCompact : public SquareCompact<Size::block>
{
 public:
  BlockSquareCompact(coordinates_type block_square) : SquareCompact<Size::block>(block_square) { }
  constexpr BlockSquareCompact(int x, int y) : SquareCompact<Size::block>(x, y) { }
};
