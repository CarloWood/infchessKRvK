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

  void mirror() { pos_.mirror(); }
};

std::ostream& operator<<(std::ostream& os, Rook const& piece);
