#pragma once

#include "Square.h"
#include "Color.h"
#include <utils/has_print_on.h>
#include <iosfwd>

// This class defines a print_on method.
using utils::has_print_on::operator<<;

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

  bool is_on_main_diagonal() const
  {
    return pos_.n == pos_.m;
  }

  void mirror() { pos_.mirror(); }

  void print_on(std::ostream& os) const;
};
