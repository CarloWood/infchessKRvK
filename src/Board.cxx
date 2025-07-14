#include "sys.h"
#include "Board.h"
#include "KingMoves.h"
#include "RookMoves.h"
#include "utils/print_using.h"
#include <string_view>
#include <iostream>
#include <iomanip>
#include <cassert>
#include "debug.h"

NAMESPACE_DEBUG_CHANNELS_START
channel_ct alternate("ALTERNATE");
NAMESPACE_DEBUG_CHANNELS_END

// Set this to true if your terminal has a dark background color.
constexpr bool white_on_black_terminal = true;
constexpr Color Black = white_on_black_terminal ? white : black;
constexpr Color White = white_on_black_terminal ? black : white;

namespace {
std::u8string_view const black_king = u8" ♚";
std::u8string_view const black_rook = u8" ♜";
std::u8string_view const black_none = u8" ·";
std::u8string_view const white_king = u8" ♔";
std::u8string_view const white_rook = u8" ♖";
std::u8string_view const white_none = u8"  ";
std::u8string_view const corner     = u8" ┏";
std::u8string_view const top_side   = u8"━";
std::u8string_view const left_side  = u8" ┃";
} // namespace

// Write std::u8string_view to an ostream as-is.
void raw_utf8(std::ostream& os, std::u8string_view const& sv)
{
  os.write(reinterpret_cast<const char*>(sv.data()), sv.size());
}

void print_none_to(std::ostream& os, Color color)
{
  os << utils::print_using(color == Black ? black_none : white_none, &raw_utf8);
}

void print_king_to(std::ostream& os, Color color)
{
  os << utils::print_using(color == Black ? black_king : white_king, &raw_utf8);
}

void print_rook_to(std::ostream& os)
{
  os << utils::print_using(white == Black ? black_rook : white_rook, &raw_utf8);
}

void Board::utf8art(std::ostream& os) const
{
  if constexpr (Board::horizontal_limit_printing > 10)
  {
    os << "\n    ";
    for (int n = 0; n < Board::horizontal_limit_printing; ++n)
    {
      int tenth = n / 10;
      os << ' ';
      if (tenth == 0)
        os << ' ';
      else
        os << (tenth % 10);
    }
  }
  os << "\n  " << utils::print_using(corner, &raw_utf8);
  for (int n = 0; n < Board::horizontal_limit_printing; ++n)
    os << utils::print_using(top_side, &raw_utf8) << (n % 10);
  os << '\n';
  // Print top to bottom.
  for (int m = 0; m < Board::vertical_limit_printing; ++m)
  {
    os << std::setw(2) << std::setfill(' ') << std::right << (m % 100) << utils::print_using(left_side, &raw_utf8);
    // Print left to right.
    for (int n = 0; n < Board::horizontal_limit_printing; ++n)
    {
      Square pos{n, m};
      if (print_flipped_)
        pos.mirror();

      if (bK_.pos() == pos)
        print_king_to(os, black);
      else if (wK_.pos() == pos)
        print_king_to(os, white);
      else if (wR_.pos() == pos)
        print_rook_to(os);
      else
        print_none_to(os, (n + m) % 2 == 1 ? black : white);
    }
    if (m == 2)
      os << "     " << *this;
    os << "\n";
  }
}

void Board::canonicalize()
{
  if (!is_canonical())
  {
    // Flip the board so that it becomes canonical.
    bK_.mirror();
    wK_.mirror();
    wR_.mirror();
    // However, make sure that the way it is printed doesn't change.
    print_flipped_ = !print_flipped_;
    Dout(dc::notice, "Board was not canonical. After mirroring: " << *this);
  }
}

bool Board::distance_less(Board const& board) const
{
  // Only compare boards that are cannonical.
  assert(board.is_canonical());

  // Note: we ignore the print_flipped_ boolean here!
  // This function is only used for ordering of a std::map,
  // and though we have links between elements of that map,
  // representing a move, it is possible to figure out what
  // that move was by reconstructing legal moves from all
  // possible flipped positions.
  //
  // For example,
  //
  //   ┏━0━1━2━3━4━        ┏━0━1━2━3━4━
  // 0 ┃ ♔   ·   ·       0 ┃ · ♔ ·   ·
  // 1 ┃   ·   ♚         1 ┃   ·   ·
  // 2 ┃ ·   · ♜ ·   ==> 2 ┃ ·   ·   ·
  // 3 ┃   ·   ·         3 ┃   ♚ ♜ ·
  // 4 ┃ ·   ·   ·       4 ┃ ·   ·   ·
  //   (black to move)     (white to move)
  //
  // means the board was flipped because the white pieces moved.
  // Moreover, because the first position has the black king on the main diagonal,
  // `print_flipped_` is guaranteed false, hence `print_flipped_` must be true for the second position.
  //
  // While,
  //
  //   ┏━0━1━2━3━4━        ┏━0━1━2━3━4━
  // 0 ┃ ♔   ·   ·       0 ┃ · ♔ ·   ·
  // 1 ┃   ·   ·         1 ┃   ·   ·
  // 2 ┃ ·   ♚   ·   ==> 2 ┃ ·   ♚   ·
  // 3 ┃   ·   ♜         3 ┃   ·   ♜
  // 4 ┃ ·   ·   ·       4 ┃ ·   ·   ·
  //   (black to move)     (white to move)
  //
  // represents two possible moves at the same time: here it is possible
  // that second position has `print_flipped_` true or false.
  //
  // A link from the second position to itself also represents a single move:
  // the move where the black king crossed the main diagonal.
  //
  // Note that every position (in the std::map) where the black king is
  // not on the main diagonal represents two positions: with `print_flipped_`
  // true or false, so even if a link represents a single move, in fact it
  // still represents two moves (just from a different starting position).
  //
  // The only links that truely represent a single move is where
  // the black king is on the main diagonal in the starting position
  // and at least one of the white pieces is not.

  if (bK_.distance_less(board.bK_))
    return true;
  if (board.bK_.distance_less(bK_))
    return false;
  if (wK_.distance_less(board.wK_))
    return true;
  if (board.wK_.distance_less(wK_))
    return false;
  if (to_play_ != board.to_play_)
    return to_play_ == black;
  return wR_.distance_less(board.wR_);
}

bool Board::is_illegal() const
{
  // Test if the position is illegal; one or more of:
  //
  // 1) The kings are next to eachother or on the same square.

  Square const& bk_pos = bK_.pos();
  Square const& wk_pos = wK_.pos();
  int dx = std::abs(bk_pos.n - wk_pos.n);
  int dy = std::abs(bk_pos.m - wk_pos.m);
  if (dx <= 1 && dy <= 1)
    return true;

  // 2) One of the kings is on the same square as the rook.

  Square const& wr_pos = wR_.pos();
  if (wr_pos == bk_pos || wr_pos == wk_pos)
    return true;

  // 3) White is to move and the black king and white rook are on the same row,
  //    and the white king is not in between them.

  if (to_play_ == white)                        // Is white to move?
  {
    if (bk_pos.m == wr_pos.m)                   // Is the black king on the same row as the white rook?
    {
      // And the white king is not in the middle?
      if (wk_pos.n != bk_pos.n)
        return true;
      if (!(std::min(bk_pos.n, wr_pos.n) < wk_pos.n && wk_pos.n < std::max(bk_pos.n, wr_pos.n)))
        return true;
    }
    else if (bk_pos.n == wr_pos.n)              // Is the black king on the same file as the white rook?
    {
      // And the white king is not in the middle?
      if (wk_pos.m != bk_pos.m)
        return true;
      if (!(std::min(bk_pos.m, wr_pos.m) < wk_pos.m && wk_pos.m < std::max(bk_pos.m, wr_pos.m)))
        return true;
    }
  }

  // Legal position.
  return false;
}

void Board::mirror()
{
  DoutEntering(dc::notice, "Board::mirror() [" << this << "]");
  bK_.mirror();
  wK_.mirror();
  wR_.mirror();
}

std::vector<Board> Board::preceding_positions() const
{
  DoutEntering(dc::notice, "Board::preceding_positions for:" << *this);

  std::vector<Board> result;

  Color to_play = to_play_.next();
  Square const cwk = wK_.pos();    // The 'c' stands for Current (or Constant).
  Square const cwr = wR_.pos();
  Square const cbk = bK_.pos();

  if (to_play == black)
  {
    // Run over all possible (legal) preceding positions that the black king could have moved from.
    int index = 0;
    for (Square bk : KingMoves{*this, black})
    {
      Board adjacent_board(bk, cwk, cwr, black);
      // Make sure we print the board correctly, relative to the original.
      if (print_flipped_)
        adjacent_board.mirror();
      Dout(dc::notice, "Adjacent board (created from index " << index << "):" << utils::print_using(adjacent_board, &Board::utf8art));
      {
#ifdef CWDEBUG
        Dout(dc::alternate, " .---Potential alternate positions:");
        NAMESPACE_DEBUG::Mark __mark;
#endif
        Square const cwk2 = adjacent_board.wK().pos();
        Square const cwr2 = adjacent_board.wR().pos();
        int index2 = 0;
        for (Square bk : KingMoves{adjacent_board, black})
        {
          Board board(bk, cwk2, cwr2, white);
          if (adjacent_board.print_flipped_)
            board.mirror();
          Dout(dc::alternate, "alternate position (created from index " << index2 << "):" << utils::print_using(board, &Board::utf8art));
          ++index2;
        }
      }
      Dout(dc::alternate, " `---End of potential alternate positions.");
      result.push_back(adjacent_board);
      ++index;
    }
  }
  else
  {
    // Run over all possible (legal) preceding positions that the white king could have moved from.
    {
      int index = 0;
      for (Square wk : KingMoves{*this, white})
      {
        result.emplace_back(cbk, wk, cwr, white);
        if (print_flipped_)
          result.back().mirror();
        Dout(dc::notice, "Adjacent board (created from index " << index << "):" << utils::print_using(result.back(), &Board::utf8art));
        ++index;
      }
    }
    // Run over all possible (legal) preceding positions that the white rook could have moved from.
    int index = 0;
    for (Square wr : RookMoves{*this})
    {
      result.emplace_back(cbk, cwk, wr, white);
      if (print_flipped_)
        result.back().mirror();
      Dout(dc::notice, "Adjacent board (created from index " << index << "):" << utils::print_using(result.back(), &Board::utf8art));
      ++index;
    }
  }

  return result;
}

#ifdef CWDEBUG
void Board::print_on(std::ostream& os) const
{
  os << '{' << bK_ << ", " << wK_ << ", " << wR_ << "," << (print_flipped_ ? " (flip)" : "") << " (" << to_play_ << " to play)" << '}';
}
#endif
