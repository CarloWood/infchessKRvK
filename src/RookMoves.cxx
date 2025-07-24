#include "sys.h"
#include "RookMoves.h"
#include "debug.h"

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
channel_ct rookmoves("ROOKMOVES");
NAMESPACE_DEBUG_CHANNELS_END
#endif

RookMoves::RookMoves(Board const& board)
{
  DoutEntering(dc::rookmoves, "RookMoves(board) for board:");
  Debug(board.debug_utf8art(dc::rookmoves));

  int const board_size = board.board_size();
  // Initialize the to-be-generated adjacent position with the final position.
  // On this board it is white is to move.
  Board adjacent_board(board_size, board.black_king(), board.white_king(), board.white_rook());
  // The current coordinates of the rook.
  auto [xc, yc] = Square::default_coordinates();
  int const cwrx = board.white_rook()[xc];
  int const cwry = board.white_rook()[yc];

  // Fill adjacent_squares_ with the squares adjacent to the current rooks square
  // that are legal (and still inside the board) if the rook on it.

  // Current white king x-coordinate.
  int cwkx = -1;                        // Impossible value.
  if (cwry == board.white_king()[yc])   // Are the white king and white rook on the same row?
    cwkx = board.white_king()[xc];

  // Move rook left or right.
  for (int dir = -1; dir <= 1; dir += 2)
    for (int wrx = cwrx + dir; wrx != cwkx && 0 < wrx && wrx < board_size; wrx += dir)
    {
      adjacent_board.set_white_rook_square({wrx, cwry});

      // Do not return positions that are illegal.
      if (adjacent_board.determine_legal(white))
        adjacent_squares_.push_back(adjacent_board.white_rook());
    }

  int cwky = -1;                        // Impossible value.
  if (cwrx == board.white_king()[xc])   // Are the white king and white rook on the same file?
    cwky = board.white_king()[yc];

  // Move the rook up or down:
  for (int dir = -1; dir <= 1; dir += 2)
    for (int wry = cwry + dir; wry != cwky && 0 < wry && wry < board_size; wry += dir)
    {
      adjacent_board.set_white_rook_square({cwrx, wry});

      // Do not return positions that are illegal.
      if (adjacent_board.determine_legal(white))
        adjacent_squares_.push_back(adjacent_board.white_rook());
    }

#ifdef CWDEBUG
  Dout(dc::rookmoves|continued_cf, "Result: ");
  char const* separator = "";
  for (int i = 0; i < adjacent_squares_.size(); ++i)
  {
    Dout(dc::continued, separator << i << ": " << adjacent_squares_[i]);
    separator = ", ";
  }
  Dout(dc::finish, ".");
#endif
}
