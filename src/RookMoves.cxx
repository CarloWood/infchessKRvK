#include "sys.h"
#include "RookMoves.h"
#include "utils/print_using.h"
#include "debug.h"

RookMoves::RookMoves(Board const& board)
{
  DoutEntering(dc::notice, "RookMoves(board) for board:" << utils::print_using(board, &Board::utf8art));

  // Get the current square of the rook.
  Square const current_pos = board.wR().pos();
  // Initialize the to-be-generated adjacent position with the final position with the other player to move.
  Board adjacent_board(board.bK().pos(), board.wK().pos(), board.wR().pos(), board.to_play().next(), false);
  // The current coordinates of the rook.
  int const cn = board.wR().pos().n;
  int const cm = board.wR().pos().m;

  // Fill adjacent_squares_ with the squares adjacent to the current rooks square
  // that are legal (and still inside the board) if the rook on it.

  int wkn = -1;                         // Impossible value.
  if (cm == board.wK().pos().m)         // Are the white king and white rook on the same row?
    wkn = board.wK().pos().n;

  // Move rook left or right.
  for (int dir = -1; dir <= 1; dir += 2)
    for (int n = cn + dir; n != wkn && 0 < n && n < Board::horizontal_limit; n += dir)
    {
      adjacent_board.set_white_rook_square({n, cm});

      // Do not return positions that are illegal.
      if (!adjacent_board.is_illegal())
        adjacent_squares_.push_back(adjacent_board.wR().pos());
    }

  int wkm = -1;                         // Impossible value.
  if (cn == board.wK().pos().n)         // Are the white king and white rook on the same file?
    wkm = board.wK().pos().m;

  // Move the rook up or down:
  for (int dir = -1; dir <= 1; dir += 2)
    for (int m = cm + dir; m != wkm && 0 < m && m < Board::vertical_limit; m += dir)
    {
      adjacent_board.set_white_rook_square({cn, m});

      // Do not return positions that are illegal.
      if (!adjacent_board.is_illegal())
        adjacent_squares_.push_back(adjacent_board.wR().pos());
    }

#ifdef CWDEBUG
  Dout(dc::notice|continued_cf, "Result: ");
  char const* separator = "";
  for (int i = 0; i < adjacent_squares_.size(); ++i)
  {
    Dout(dc::continued, separator << i << ": " << adjacent_squares_[i]);
    separator = ", ";
  }
  Dout(dc::finish, ".");
#endif
}
