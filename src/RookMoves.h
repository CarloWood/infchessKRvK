#pragma once

#include "Board.h"
#include <vector>

class RookMoves
{
 public:
  using adjacent_squares_type = std::vector<Square>;
  using adjacent_squares_iterator = adjacent_squares_type::const_iterator;

 private:
  adjacent_squares_type adjacent_squares_;

 public:
  RookMoves(Board const& board);

  adjacent_squares_iterator begin() { return adjacent_squares_.begin(); }
  adjacent_squares_iterator end() { return adjacent_squares_.end(); }
};
