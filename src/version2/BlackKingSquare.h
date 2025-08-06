#pragma once

#include "KingSquare.h"

class BlackKingSquare : public KingSquare
{
 public:
  BlackKingSquare(coordinates_type square) : KingSquare(square) { }
  constexpr BlackKingSquare(int x, int y) : KingSquare(x, y) { }
};
