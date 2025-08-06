#include "sys.h"
#include "Square.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, Square const& square)
{
  using namespace coordinates;
  return os << '(' << square[x] << ", " << square[y] << ')';
}

