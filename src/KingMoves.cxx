#include "sys.h"
#include "KingMoves.h"
#include <iostream>
#include <cassert>
#include "debug.h"

NAMESPACE_DEBUG_CHANNELS_START
channel_ct illegal("ILLEGAL");
channel_ct kingmoves("KINGMOVES");
NAMESPACE_DEBUG_CHANNELS_END

KingMoves::KingMoves(Board const& board, Color color)
{
  DoutEntering(dc::kingmoves, "KingMoves(board, " << color << ") for board:");
  Debug(board.debug_utf8art(dc::kingmoves));

  // The board must already be canonicalized.
  ASSERT(board.is_canonical());

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

  bool const only_canonical = board.bK().is_on_main_diagonal() && board.wK().is_on_main_diagonal() && board.wR().is_on_main_diagonal();

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
      // Only generate canonical positions.
      if (only_canonical && dn - dm < 0)
        continue;

      Square adjacent_king_square{current_pos.n + dn, current_pos.m + dm};
      if (color == black)
        adjacent_board.set_black_king_square(adjacent_king_square);
      else
        adjacent_board.set_white_king_square(adjacent_king_square);

      // As a special case, allow a position where the black king and the white rook are
      // on the same square, with white to move. This just means that the black king just
      // took the white rook.
      if (color == black && adjacent_board.wR().pos() == adjacent_king_square)
      {
        Dout(dc::illegal, "Adding position where the black king just took the white rook on " << adjacent_king_square);
        adjacent_squares_.push_back(adjacent_king_square);
        continue;
      }

      // Do not return positions that are illegal.
      if (adjacent_board.is_illegal())
        continue;

      Dout(dc::illegal, "The following board is not illegal:");
      Debug(adjacent_board.debug_utf8art(dc::illegal));

      Dout(dc::illegal, "  (added to index " << adjacent_squares_.size() << ")");
      // Note that adjacent_king_square might be NOT canonical!
      Dout(dc::kingmoves, "Adding position where the black king moved to " << adjacent_king_square);
      adjacent_squares_.push_back(adjacent_king_square);
    }
  }

#ifdef CWDEBUG
  Dout(dc::kingmoves|continued_cf, "Result: ");
  char const* separator = "";
  for (int i = 0; i < adjacent_squares_.size(); ++i)
  {
    Dout(dc::continued, separator << i << ": " << adjacent_squares_[i]);
    separator = ", ";
  }
  Dout(dc::finish, ".");
#endif
}
