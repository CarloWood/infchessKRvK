#include "BlackKing.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, BlackKing const& piece)
{
  os << "black king:" << piece.pos();
  return os;
}
