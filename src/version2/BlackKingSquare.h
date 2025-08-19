#pragma once

#include "KingSquare.h"

class BlackKingSquare : public KingSquare
{
 public:
  BlackKingSquare(coordinates_type square) : KingSquare(square) { }
  BlackKingSquare(int x, int y) : KingSquare(x, y) { }
};
