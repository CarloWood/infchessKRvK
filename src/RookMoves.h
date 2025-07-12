#pragma once

#include "Board.h"
#include <vector>

class RookMoves
{
 public:
  using preceding_squares_type = std::vector<Square>;
  using preceding_squares_iterator = preceding_squares_type::const_iterator;

 private:
  preceding_squares_type preceding_squares_;

 public:
  RookMoves(Board const& board);

  preceding_squares_iterator begin() { return preceding_squares_.begin(); }
  preceding_squares_iterator end() { return preceding_squares_.end(); }
};
