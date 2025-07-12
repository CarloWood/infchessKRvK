#include "sys.h"
#include "WhiteKing.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, WhiteKing const& piece)
{
  os << "white king:" << piece.pos();
  return os;
}
