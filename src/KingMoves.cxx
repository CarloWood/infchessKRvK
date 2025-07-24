#include "sys.h"
#include "KingMoves.h"
#include <iostream>
#include <cassert>
#include "debug.h"

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
channel_ct illegal("ILLEGAL");
channel_ct kingmoves("KINGMOVES");
NAMESPACE_DEBUG_CHANNELS_END
#endif

KingMoves::KingMoves(Board const& board, Color color, bool forward)
{
  DoutEntering(dc::kingmoves, "KingMoves(board, " << color << ", forward:" << std::boolalpha << forward << ") for board:");
  Debug(board.debug_utf8art(dc::kingmoves));

  // black king:(1, 0), white king:(2, 2), white rook:(0, 1)
  if (color == black && board.black_king() == Square{1, 0} && board.white_king() == Square{2, 2} && board.white_rook() == Square{0, 1})
  {
    Dout(dc::notice, "start");
  }

  // Get the current square of the king.
  Square const current_pos = color == black ? board.black_king() : board.white_king();
  int const board_size = board.board_size();
  // Initialize the to-be-generated adjacent position with the target position.
  // Note that on (this) Board it is the turn of the color opposite to the color of
  // the king we are moving if we are generating preceding positions: after that king
  // moved it is the turn of the other player. While, if we are generating next
  // positions then on (this) Board it is the turn of the color equal to the color of
  // the king we are moving: otherwise it wouldn't be legal to move it ;).
  Board adjacent_board(board);
  // adjacent_board is generated below. The color that is to move in that position
  //
  //  |                                 forward
  //  +---------------------------------------------------------------------------------
  //  |               false                |                 true
  //  +------------------------------------+--------------------------------------------
  //  | adjacent_board (`color` to move)   |
  //  |     | `color` king is moved.       |
  //  |     v                              |
  //  | board (`color.opponent()` to move) | board (`color` to move)
  //  |                                    |   | `color` king is moved.
  //  |                                    |   v
  //  |                                    | adjacent_board (`color.opponent()` to move)
  Color adjacent_to_move = forward ? color.opponent() : color;

  // Fill adjacent_squares_ with the squares adjacent to the current `color` kings square
  // that are legal (and still inside the board, unless forward is true) if the king was on it.
  for (int dx = -1; dx <= 1; ++dx)
  {
    using namespace coordinates;

    // Don't generate positions where the king is outside the board.
    if (current_pos[x] == 0 && dx == -1)
      continue;
    // Don't generate positions where the king is beyond the horizontal_limit, unless we're generating forward moves.
    if (!forward && current_pos[x] == board_size - 1 && dx == 1)
      continue;
    for (int dy = -1; dy <= 1; ++dy)
    {
      // It must be a move.
      if (dx == 0 && dy == 0)
        continue;
      // Don't generate positions where the king is outside the board.
      if (current_pos[y] == 0 && dy == -1)
        continue;
      // Don't generate positions where the king is beyond the vertical_limit, unless we're generating forward moves.
      if (!forward && current_pos[y] == board_size - 1 && dy == 1)
        continue;

      Square adjacent_king_square{current_pos[x] + dx, current_pos[y] + dy};
      if (color == black)
        adjacent_board.set_black_king_square(adjacent_king_square);
      else
        adjacent_board.set_white_king_square(adjacent_king_square);

      // It is not possible that the preceding position has the black king and white rook on the same square.
      if (!forward && adjacent_board.white_rook() == adjacent_king_square)
        continue;

      // As a special case, allow a position where the black king and the white rook are
      // on the same square, with white to move. This just means that the black king just
      // took the white rook. Only if the rook isn't protected by the white king of course.
      if (forward && color == black &&
          adjacent_board.white_rook() == adjacent_king_square && !adjacent_king_square.is_next_to(adjacent_board.white_king()))
      {
        Dout(dc::illegal, "Adding position where the black king just took the white rook on " << adjacent_king_square);
        adjacent_squares_.push_back(adjacent_king_square);
        continue;
      }

      // Do not return positions that are illegal.
      if (!adjacent_board.determine_legal(adjacent_to_move))
        continue;

      Dout(dc::illegal, "The following board is not illegal:");
      Debug(adjacent_board.debug_utf8art(dc::illegal));

      Dout(dc::illegal, "  (added to index " << adjacent_squares_.size() << ")");
      // Note that adjacent_king_square might be NOT canonical!
      Dout(dc::kingmoves, "Adding position where the black king " << (forward ? "moved to" : "came from") << " " << adjacent_king_square);
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
