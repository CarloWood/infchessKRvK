#include "KingMoves.h"
#include <iostream>
#include <cassert>

KingMoves::KingMoves(Board const& board, Color color)
{
  std::cout << "Entering KingMoves(board, " << color << ") for board:\n";
  board.print();

  // The board must already be canonicalized.
  assert(board.is_canonical());

  // Get the current square of the king.
  Square const current_pos = color == black ? board.bK().pos() : board.wK().pos();
  // Initialize the to-be-generated adjacent position with the target position.
  // Note that on (this) Board it is the turn of the color opposite to the color of
  // the king we are moving if are generating preceding positions: after that king
  // moved it is the turn of the other player. While, if we are generating next
  // positions then on (this) Board it is the turn of the color equal to the color of
  // the king we are moving: otherwise it wouldn't be legal to move it ;).
  Board adjacent_board(board);
  adjacent_board.null_move();

  // Fill adjacent_squares_ with the squares adjacent to the current `color` kings square
  // that are legal (and still inside the board) if the king was on it.
  for (int dn = -1; dn <= 1; ++dn)
  {
    // Don't generate positions where the king is outside the board.
    if (current_pos.n == 0 && dn == -1)
      continue;
    // Don't generate positions where the king is beyond the horizontal_limit.
    if (current_pos.n == Board::horizontal_limit - 1 && dn == 1)
      continue;
    for (int dm = -1; dm <= 1; ++dm)
    {
      // It must be a move.
      if (dn == 0 && dm == 0)
        continue;
      // Don't generate positions where the king is outside the board.
      if (current_pos.m == 0 && dm == -1)
        continue;
      // Don't generate positions where the king is beyond the vertical_limit.
      if (current_pos.m == Board::vertical_limit - 1 && dm == 1)
        continue;

      Square adjacent_king_square{current_pos.n + dn, current_pos.m + dm};
      if (color == black)
        adjacent_board.set_black_king_square(adjacent_king_square);
      else
        adjacent_board.set_white_king_square(adjacent_king_square);

      // Do not return positions that are illegal.
      if (adjacent_board.is_illegal())
        continue;

      std::cout << "The following board is not illegal:\n";
      adjacent_board.print();

      std::cout << "  (added to index " << adjacent_squares_.size() << ")\n";
      // Note that adjacent_king_square might be NOT canonical!
      adjacent_squares_.push_back(adjacent_king_square);
    }
  }

  std::cout << "Leaving KingMoves\n";
}
