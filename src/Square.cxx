#include "Square.h"
#include "Board.h"
#include <iostream>
#include <algorithm>

bool Square::distance_less(Square const& rhs_pos) const
{
  int sum = n + m;
  int other_sum = rhs_pos.n + rhs_pos.m;

  if (sum < other_sum)
    return true;
  if (other_sum < sum)
    return false;

  // We ignore `print_flipped_`, see Board::distance_less.
  return m < rhs_pos.m;
}

std::ostream& operator<<(std::ostream& os, Square pos)
{
  return os << '(' << pos.n << ", " << pos.m << ')';
}
