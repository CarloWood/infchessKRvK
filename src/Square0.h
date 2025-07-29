#pragma once

#include "coordinates.h"
#include <iosfwd>
#include <utility>
#include <array>
#include <cmath>

namespace version0 {

class Square
{
 private:
  // (0, 0) means the piece is in the (only) corner of the quarter-infinite board.
  std::array<int, 2> coordinates_{};

 public:
  Square() = default;
  Square(int x, int y) : coordinates_{x, y} { }

  static constexpr std::pair<int, int> default_coordinates()
  {
    using namespace coordinates;
    return {x, y};
  }

  // Used for ordering of a std::map with Board's.
  bool distance_less(Square const& rhs_square) const
  {
    using namespace coordinates;

    int lhs_sum = coordinates_[x] + coordinates_[y];
    int rhs_sum = rhs_square[x] + rhs_square[y];

    return (lhs_sum != rhs_sum) ? lhs_sum < rhs_sum : coordinates_[y] < rhs_square[y];
  }

  // Accessor for the individual coordinates.
  int operator[](int index) const
  {
    return coordinates_[index];
  }

  bool is_next_to(Square const& pos) const
  {
    using namespace coordinates;
    return std::abs(coordinates_[x] - pos[x]) <= 1 && std::abs(coordinates_[y] - pos[y]) <= 1;
  }

  friend bool operator==(Square const& lhs, Square const& rhs)
  {
    return lhs.coordinates_ == rhs.coordinates_;
  }
};

std::ostream& operator<<(std::ostream& os, Square const& square);

} // namespace version0
