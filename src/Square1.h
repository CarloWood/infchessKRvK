#pragma once

#include "Board1.h"
#include "coordinates.h"
#include <iosfwd>
#include <utility>
#include <array>
#include <cmath>

namespace version1 {

class Square : public SquareCompact
{
 public:
  Square() = default;
  constexpr Square(int x, int y) : SquareCompact(x, y) { }
  Square(SquareCompact coordinates) : SquareCompact(coordinates) { }

  static constexpr std::pair<int, int> default_coordinates()
  {
    using namespace coordinates;
    return {x, y};
  }

  // Used for ordering of a std::map with Board's.
  bool distance_less(Square const& rhs_square) const
  {
    return coordinates_ < rhs_square.coordinates_;
  }

  // Accessor for the individual coordinates.
  int operator[](int index) const
  {
    std::array<int, 2> xy_coordinates = {Board::x_coord(coordinates_), Board::y_coord(coordinates_)};
    return xy_coordinates[index];
  }

  bool is_next_to(Square const& pos) const
  {
    using namespace coordinates;
    return std::abs(Board::x_coord(coordinates_) - pos[x]) <= 1 && std::abs(Board::y_coord(coordinates_) - pos[y]) <= 1;
  }

  friend bool operator==(Square const& lhs, Square const& rhs)
  {
    return lhs.coordinates_ == rhs.coordinates_;
  }
};

std::ostream& operator<<(std::ostream& os, Square const& square);

} // namespace version1
