#pragma once

#include "coordinates.h"
#include "BlackKingSquare.h"
#include "WhiteKingSquare.h"
#include "WhiteRookSquare.h"
#include <iosfwd>
#include <utility>
#include <array>
#include <cmath>

// Non-compact coordinates of a board square, suitable for any piece.
class Square
{
 private:
  std::array<int, 2> xy_coordinates_{};

 public:
  Square() = default;
  constexpr Square(int x, int y) : xy_coordinates_{x, y} { }
  Square(BlackKingSquare square) : xy_coordinates_{square.x_coord(), square.y_coord()} { }
  Square(WhiteKingSquare square) : xy_coordinates_{square.x_coord(), square.y_coord()} { }
  Square(WhiteRookSquare square) : xy_coordinates_{square.x_coord(), square.y_coord()} { }

  static constexpr std::tuple<int, int, int, int> default_coordinates()
  {
    using namespace coordinates;
    return {x, y, Size::board::x, Size::board::y};
  }

  // Accessor for the individual coordinates.
  int operator[](int index) const
  {
    return xy_coordinates_[index];
  }

  bool is_next_to(Square const& pos) const
  {
    using namespace coordinates;
    return std::abs(xy_coordinates_[x] - pos[x]) <= 1 && std::abs(xy_coordinates_[y] - pos[y]) <= 1;
  }

  friend bool operator==(Square const& lhs, Square const& rhs)
  {
    using namespace coordinates;
    return lhs.xy_coordinates_ == rhs.xy_coordinates_;
  }
};

std::ostream& operator<<(std::ostream& os, Square const& square);
