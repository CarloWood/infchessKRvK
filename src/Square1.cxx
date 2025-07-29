#include "sys.h"
#include "Square1.h"
#include <iostream>

namespace version1 {

std::ostream& operator<<(std::ostream& os, Square const& square)
{
  using namespace coordinates;
  return os << '(' << square[x] << ", " << square[y] << ')';
}

} // namespace version1
