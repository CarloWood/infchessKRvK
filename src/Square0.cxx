#include "sys.h"
#include "Square0.h"
#include <iostream>

namespace version0 {

std::ostream& operator<<(std::ostream& os, Square const& square)
{
  using namespace coordinates;
  return os << '(' << square[x] << ", " << square[y] << ')';
}

} // namespace version0
