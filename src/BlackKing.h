#pragma once

#include "Square.h"
#include "Color.h"
#include <iosfwd>

class BlackKing
{
 private:
  Square pos_;

 public:
  BlackKing() = default;
  BlackKing(Square pos, Color color) : pos_(pos) { }

  // Accessors.
  Square const& pos() const { return pos_; }
  Square& pos() { return pos_; }

  bool distance_less(BlackKing const& king) const
  {
    return pos_.distance_less(king.pos_);
  }

  bool is_canonical() const
  {
    // The black king is in the upper-right triangle (including the main diagonal).
    return pos_.is_canonical();
  }

  void mirror() { pos_.mirror(); }
};

std::ostream& operator<<(std::ostream& os, BlackKing const& piece);
