#include "sys.h"
#include "Rook.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, Rook const& piece)
{
  os << "rook:" << piece.pos();
  return os;
}
