#include "sys.h"
#include "Square.h"
#include <iostream>

bool Square::distance_less(Square const& rhs_square) const
{
  using namespace coordinates;

  int lhs_sum = coordinates_[x] + coordinates_[y];
  int rhs_sum = rhs_square[x] + rhs_square[y];

  return (lhs_sum != rhs_sum) ? lhs_sum < rhs_sum : coordinates_[y] < rhs_square[y];
}

std::ostream& operator<<(std::ostream& os, Square const& square)
{
  using namespace coordinates;
  return os << '(' << square[x] << ", " << square[y] << ')';
}
