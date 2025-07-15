#include "sys.h"
#include "Board.h"
#include "utils/print_using.h"
#include <string_view>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <sstream>
#include "debug.h"

NAMESPACE_DEBUG_CHANNELS_START
channel_ct board("BOARD");
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

Board::Board(Square bk, Square wk, Square wr, Color to_play, bool mirror) : bK_(bk, black), wK_(wk, white), wR_(wr), to_play_(to_play)
{
  DoutEntering(dc::board, "Board(" << bk << ", " << wk << ", " << wr << ", " << to_play << ") [" << this << "]");
  Dout(dc::board, "Board [" << this << "] is now: " << *this);
  // Canonicalize the board and optionally mirror the position.
  canonicalize(mirror);
}

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

#ifdef CWDEBUG
void Board::debug_utf8art(libcwd::channel_ct const& debug_channel) const
{
  if (!debug_channel.is_on())
    return;

  if constexpr (Board::horizontal_limit_printing > 10)
  {
    Dout(debug_channel|continued_cf, "    ");
    for (int n = 0; n < Board::horizontal_limit_printing; ++n)
    {
      int tenth = n / 10;
      Dout(dc::continued, ' ');
      if (tenth == 0)
        Dout(dc::continued, ' ');
      else
        Dout(dc::continued, (tenth % 10));
    }
    Dout(dc::finish, "");
  }
  Dout(debug_channel|continued_cf, "  " << utils::print_using(corner, &raw_utf8));
  for (int n = 0; n < Board::horizontal_limit_printing; ++n)
    Dout(dc::continued, utils::print_using(top_side, &raw_utf8) << (n % 10));
  Dout(dc::finish, "");
  // Print top to bottom.
  for (int m = 0; m < Board::vertical_limit_printing; ++m)
  {
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill(' ') << std::right << (m % 100) << utils::print_using(left_side, &raw_utf8);
    // Print left to right.
    for (int n = 0; n < Board::horizontal_limit_printing; ++n)
    {
      Square pos{n, m};
      if (print_flipped_)
        pos.mirror();

      if (bK_.pos() == pos)
        print_king_to(oss, black);
      else if (wK_.pos() == pos)
        print_king_to(oss, white);
      else if (wR_.pos() == pos)
        print_rook_to(oss);
      else
        print_none_to(oss, (n + m) % 2 == 1 ? black : white);
    }
    if (m == 2)
      oss << "     " << *this;
    Dout(debug_channel, oss.str());
  }
}
#endif

void Board::canonicalize(bool mirror)
{
  DoutEntering(dc::board, "Board::canonicalize(" << std::boolalpha << mirror << ")");

  // A board can be 'canonical' (the black king has stored coordinates (n, m) where m <= n), or not.
  // If the black king is on the main diagonal then print_flipped_ should be off after canonicalizing it.

  // Possible cases:
  // 1) The board is canonical with the black king not on the main diagonal and we do not want to mirror it.
  //    Do nothing.
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ · ♔ · ♚       0 ┃ · ♔ · ♚
  //     1 ┃   ·   ·  ==>  1 ┃   ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   · ♜
  //     3 ┃   ·   ·       3 ┃   ·   ·
  //     false             false
  //
  // or
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ ·   · ♚       0 ┃ ·   · ♚
  //     1 ┃ ♔ ·   ·  ==>  1 ┃ ♔ ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   · ♜
  //     3 ┃   ·   ·       3 ┃   ·   ·
  //     true              true

  // 2) The board is canonical with the black king not on the main diagonal and we do want to mirror it.
  //    Toggle print_flipped_.
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ · ♔ · ♚       0 ┃ ·   ·
  //     1 ┃   ·   ·  ==>  1 ┃ ♔ ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   ·
  //     3 ┃   ·   ·       3 ┃ ♚ · ♜ ·
  //     false             true
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ ·   · ♚       0 ┃ · ♔ ·
  //     1 ┃ ♔ ·   ·  ==>  1 ┃   ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   ·
  //     3 ┃   ·   ·       3 ┃ ♚ · ♜ ·
  //     true              false

  // 3a) The black king is on the main diagonal and the flipped bit is off and we do not want to mirror it.
  //     Do nothing.
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ ♔   · ♚       0 ┃ ♔   · ♚
  //     1 ┃   ·   ·  ==>  1 ┃   ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   · ♜
  //     3 ┃   ·   ·       3 ┃   ·   ·
  //     false             false

  // 3b) The black king is on the main diagonal but the flipped bit is on and we do not want to mirror it.
  //     Mirror wK_ and wR_ (and optionally bK_) and
  //     set print_flipped_ to false (i.e. toggle it).
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ ♔   · ♚       0 ┃ ♔   · ♚
  //     1 ┃   ·   ·  ==>  1 ┃   ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   · ♜
  //     3 ┃   ·   ·       3 ┃   ·   ·
  //     true(?!)          false

  // 4a) The black king is on the main diagonal and the flipped bit is off and we do want to mirror it.
  //     Mirror wK_ and wR_ (and optionally bK_).
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ ♔   · ♚       0 ┃ ♔   ·
  //     1 ┃   ·   ·  ==>  1 ┃   ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   ·
  //     3 ┃   ·   ·       3 ┃ ♚ · ♜ ·
  //     false             false

  // 4b) The black king is on the main diagonal but the flipped bit is on and we do want to mirror it.
  //     Set print_flipped_ to false (i.e. toggle it).
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ ♔   · ♚       0 ┃ ♔   ·
  //     1 ┃   ·   ·  ==>  1 ┃   ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   ·
  //     3 ┃   ·   ·       3 ┃ ♚ · ♜ ·
  //     true(?!)          false

  // 5) The board is not canonical and we do not want to mirror it.
  //    Mirror wK_, wR_ and bK_ and
  //    toggle print_flipped_.
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ ·   · ♚       0 ┃ ·   · ♚
  //     1 ┃ ♔ ·   ·  ==>  1 ┃ ♔ ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   · ♜
  //     3 ┃   ·   ·       3 ┃   ·   ·
  //     false             true
  //
  // or
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ · ♔ · ♚       0 ┃ · ♔ · ♚
  //     1 ┃   ·   ·  ==>  1 ┃   ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   · ♜
  //     3 ┃   ·   ·       3 ┃   ·   ·
  //     true              false

  // 6) The board is not canonical and we do want to mirror it.
  //    Mirror wK_, wR_ and bK_
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ ·   · ♚       0 ┃ · ♔ ·
  //     1 ┃ ♔ ·   ·  ==>  1 ┃   ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   ·
  //     3 ┃   ·   ·       3 ┃ ♚ · ♜ ·
  //     false             false
  //
  // or
  //
  //       ┏━0━1━2━3         ┏━0━1━2━3
  //     0 ┃ · ♔ · ♚       0 ┃ ·   ·
  //     1 ┃   ·   ·  ==>  1 ┃ ♔ ·   ·
  //     2 ┃ ·   · ♜       2 ┃ ·   ·
  //     3 ┃   ·   ·       3 ┃ ♚ · ♜ ·
  //     true              true

  bool const canonical = is_canonical();
  bool const bk_on_main_diagonal = bK_.is_on_main_diagonal();

  // Thus print_flipped_ must be toggled in these cases:
  bool do_flip =
  // 2) The board is canonical with the black king not on the main diagonal and we do want to mirror it.
    (!bk_on_main_diagonal && canonical && mirror) ||
  // 3b+4b) The black king is on the main diagonal but the flipped bit is on.
    (bk_on_main_diagonal && print_flipped_) ||
  // 5) The board is not canonical and we do not want to mirror it.
    (!canonical && !mirror);

  // While mirroring must happen in these cases:
  bool do_mirror =
  // 3b+4a) The black king is on the main diagonal and print_flipped_ != mirror.
    (bk_on_main_diagonal && print_flipped_ != mirror) ||
  // 5+6) The board is not canonical.
    !canonical;

  if (do_mirror)
  {
    bK_.mirror();
    wK_.mirror();
    wR_.mirror();
  }
  if (do_flip)
  {
    print_flipped_ = !print_flipped_;
  }
#ifdef CWDEBUG
  if (do_mirror || do_flip)
    Dout(dc::board, "Board was changed: " << *this);
#endif

  // The result must always be cannonical.
  ASSERT(is_canonical());
  // King did not move from or onto the main diagonal.
  ASSERT(bk_on_main_diagonal == bK_.is_on_main_diagonal());
  // If the king is on the main diagonal, then print_flipped_ should be off.
  ASSERT(!bk_on_main_diagonal || !print_flipped_);
#if CW_DEBUG
  bool const position_was_mirrored = do_mirror != do_flip;
  // The position was mirrored only if mirror is true.
  ASSERT(mirror == position_was_mirrored);
#endif
}

bool Board::distance_less(Board const& board) const
{
  // Only compare boards that are cannonical.
  ASSERT(board.is_canonical());

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
  DoutEntering(dc::board, "Board::mirror() [" << this << "]");
  bK_.mirror();
  wK_.mirror();
  wR_.mirror();
}

#ifdef CWDEBUG
void Board::print_on(std::ostream& os) const
{
  os << '{' << bK_ << ", " << wK_ << ", " << wR_ << "," << (print_flipped_ ? " (flip)" : "") << " (" << to_play_ << " to play)" << '}';
}
#endif
