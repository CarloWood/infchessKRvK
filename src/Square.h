#pragma once

#include <iosfwd>
#include <algorithm>

// The square that a piece occupies (king or rook).
struct Square
{
  // 0, 0 means the piece is in the (only) corner of the quarter-infinite board.
  int n;        // Horizontal distance to the left-edge. 0 means against the edge.
  int m;        // Vertical distance to the top-edge. 0 means against the edge.

  Square() : n(0), m(0) { }
  Square(int n1, int m1) : n(n1), m(m1) { }

  bool distance_less(Square const& pos) const;

  // This function only makes sense when called on the square that the black king is on.
  bool is_canonical() const
  {
    // Is in the upper-right triangle (including main diagonal)?
    return m <= n;
  }

  void mirror()
  {
    std::swap(n, m);
  }
};

std::ostream& operator<<(std::ostream& os, Square pos);

inline bool operator==(Square const& lhs, Square const& rhs)
{
  return lhs.n == rhs.n && lhs.m == rhs.m;
}
