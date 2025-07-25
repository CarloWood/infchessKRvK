#pragma once

#include <iosfwd>
#include <utility>
#include <array>
#include <cmath>

namespace coordinates {

// By default,
// index 0 contains the horizontal distance to the left-edge (0 means against the edge).
// index 1 contains the vertical distance to the top-edge (0 means against the edge).
static constexpr int x = 0;
static constexpr int y = 1;

} // namespace coordinates

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
  bool distance_less(Square const& pos) const;

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
