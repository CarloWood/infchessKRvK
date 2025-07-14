#include "sys.h"
#include "BlackKing.h"
#include <iostream>

void BlackKing::print_on(std::ostream& os) const
{
  os << "black king:" << pos_;
}
