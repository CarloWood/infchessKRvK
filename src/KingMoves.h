#pragma once

#include "Board.h"
#include <vector>

// Generate 'adjacent' squares that the king can reach in one ply, or potentially came from, the previous ply.
// Squares where the black king is in check with white to move (an illegal position) are not returned.
class KingMoves
{
 public:
  using adjacent_squares_type = std::vector<Square>;
  using adjacent_squares_iterator = adjacent_squares_type::const_iterator;

 private:
  adjacent_squares_type adjacent_squares_;

 public:
  KingMoves(Board const& board, Color color);

  adjacent_squares_iterator begin() { return adjacent_squares_.begin(); }
  adjacent_squares_iterator end() { return adjacent_squares_.end(); }
};
