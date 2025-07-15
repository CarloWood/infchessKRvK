#pragma once

#include "Square.h"
#include <iosfwd>

// The white rook.
class Rook
{
 private:
  Square pos_;

 public:
  Rook() = default;
  Rook(Square pos) : pos_(pos) { }

  // Accessor.
  Square const& pos() const { return pos_; }
  Square& pos() { return pos_; }

  bool distance_less(Rook const& rook) const
  {
    return pos_.distance_less(rook.pos_);
  }

  bool is_canonical() const
  {
    // The rook king is in the upper-right triangle (including the main diagonal).
    return pos_.is_canonical();
  }

  bool is_on_main_diagonal() const
  {
    return pos_.n == pos_.m;
  }

  void mirror() { pos_.mirror(); }
};

std::ostream& operator<<(std::ostream& os, Rook const& piece);
