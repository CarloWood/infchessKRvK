#pragma once

#include "KingSquare.h"

class WhiteKingSquare : public KingSquare
{
 public:
  WhiteKingSquare(coordinates_type square) : KingSquare(square) { }
  constexpr WhiteKingSquare(int x, int y) : KingSquare(x, y) { }
};

