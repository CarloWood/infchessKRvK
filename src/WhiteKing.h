#pragma once

#include "Square.h"
#include "Color.h"
#include <iosfwd>

class WhiteKing
{
 private:
  Square pos_;

 public:
  WhiteKing() = default;
  WhiteKing(Square pos, Color color) : pos_(pos) { }

  // Accessors.
  Square const& pos() const { return pos_; }
  Square& pos() { return pos_; }

  bool distance_less(WhiteKing const& white_king) const
  {
    return pos_.distance_less(white_king.pos_);
  }

  void mirror() { pos_.mirror(); }
};

std::ostream& operator<<(std::ostream& os, WhiteKing const& piece);
