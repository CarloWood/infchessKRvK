#include "sys.h"
#include "Board.h"
#include "Color.h"
#include "utils/print_using.h"
#include <utility>
#include <cmath>
#include <string_view>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "debug.h"

// Implementation written and tested by Carlo Wood - 2025/07/19.
//
// Note: this function assumes it is black to move, therefore it should
// only be called when black is to move.
//
bool Board::black_has_moves() const
{
  // Set up coordinates for easy swapping.
  auto [x, y, board_size_x, board_size_y] = Square::default_coordinates();
  auto [bk, wk, wr] = abbreviations();

  // If the black king is to move but isn't mate or stalemate then black can move.
  // It can only be mate or stalemate at the edge of the board.
  if (bk[x] > 0 && bk[y] > 0)
    return true;

  // The black king is now against the edge.
  //       ┏━0━1━2━3
  //     0 ┃ ♔ ♔ ♔ ♔
  //     1 ┃ ♔ ·   ·
  //     2 ┃ ♔   ·
  //     3 ┃ ♔ ·   ·

  // To simply the code below, lets flip the position if the king is not against the left edge.
  if (bk[x] != 0)
  {
    std::swap(x, y);
    std::swap(board_size_x, board_size_y);
  }

  // The black king is now against the left edge.
  //       ┏━0━1━2━3
  //     0 ┃ ♔   ·
  //     1 ┃ ♔ ·   ·
  //     2 ┃ ♔   ·
  //     3 ┃ ♔ ·   ·

  if (bk[y] != 0)
  {
    // If the king is not in the corner, then it can't be stalemate.
    // It can only be mate if the white king is opposite of the black king.
    if (wk[x] != 2 || wk[y] != bk[y])
      return true;

    // The black king is now against the left edge, but not in the corner,
    // and the white king is opposing it.
    //       ┏━0━1━2━3        ┏━0━1━2━3        ┏━0━1━2━3
    //     0 ┃ ·   ·        0 ┃ ·   ·        0 ┃ ·   ·
    //     1 ┃ ♔ · ♚ ·  or  1 ┃   ·   ·  or  1 ┃   ·   ·
    //     2 ┃ ·   ·        2 ┃ ♔   ♚        2 ┃ ·   ·
    //     3 ┃   ·   ·      3 ┃   ·   ·      3 ┃ ♔ · ♚ ·

    // Still, the black king can move unless it is mate.
    // It can only be mate if the white rook is also against the left edge.
    if (wr[x] != 0)
      return true;

    // Finally, it is mate unless the rook can be captured, or the king already captured the rook.
    return std::abs(bk[y] - wr[y]) <= 1;
  }

  // The black king is now in the corner.
  //       ┏━0━1━2━3
  //     0 ┃ ♔   ·
  //     1 ┃   ·   ·
  //     2 ┃ ·   ·
  //     3 ┃   ·   ·

  // We don't really care about whether or not black is in check, we only
  // need to know if (1, 0), (1, 1) and (0, 1) are attacked by white.
  //
  // Note that the rook can't be on (0, 0) too as it is black to move.

  // If this is to be mate or stalemate then the white king must attack (1, 1).
  // After all, the positions left that are mate or stalemate are one of:
  //       ┏━0━1━2━3     ┏━0━1━2━3     ┏━0━1━2━3     ┏━0━1━2━3     ┏━0━1━2━3
  //     0 ┃ ♔   ♚     0 ┃ ♔   ·     0 ┃ ♔   ·     0 ┃ ♔   ♜ ♜   0 ┃ ♔   ♜ ♜
  //     1 ┃   ♜ ♜ ♜   1 ┃   ♜ ♚ ·   1 ┃   ♜   ·   1 ┃   ♜   ·   1 ┃   ♜   ·
  //     2 ┃ ♜   ·     2 ┃ ♜   ·     2 ┃ ·   ♚     2 ┃ · ♚ ·     2 ┃ ♚ ♜ ·
  //     3 ┃ ♜ ·   ·,  3 ┃ ♜ ·   ·,  3 ┃   ·   ·,  3 ┃   ·   ·,  3 ┃   ♜   ·

  // Thus the king must be on (2, 0), (2, 1), (2, 2), (1, 2) or (0, 2).
  if (wk[x] > 2 || wk[y] > 2)
    return true;

  // We assume this is a legal position (otherwise don't call this function).
  // Therefore, the white king is now on:
  //       ┏━0━1━2━3
  //     0 ┃ ♔   ♚
  //     1 ┃   · ♚ ·
  //     2 ┃ ♚ ♚ ♚
  //     3 ┃   ·   ·

  // Therefore, it is always stalemate if the white rook is on (1, 1).
  if (wr[x] == 1 && wr[y] == 1)
    return false;

  // The only positions left to check are now:
  //   A:  ┏━0━1━2━3   B:  ┏━0━1━2━3   C:  ┏━0━1━2━3   D:  ┏━0━1━2━3
  //     0 ┃ ♔   ♚       0 ┃ ♔   ·       0 ┃ ♔   ♜ ♜     0 ┃ ♔   ♜ ♜
  //     1 ┃   · ♜ ♜     1 ┃   · ♚ ·     1 ┃   ·   ·     1 ┃   ·   ·
  //     2 ┃ ♜   ·       2 ┃ ♜   ·       2 ┃ · ♚ ·       2 ┃ ♚ ♜ ·
  //     3 ┃ ♜ ·   ·,    3 ┃ ♜ ·   ·,    3 ┃   ·   ·,    3 ┃   ♜   ·

  if (wk[y] == 0)       // A
    return !((wr[x] == 0 && wr[y] > 1) || (wr[x] > 0 && wr[y] == 1));
  else if (wk[y] == 1)  // B
    return !(wr[x] == 0 && wr[y] > 1);
  else if (wk[x] == 1)  // C
    return !(wr[x] > 1 && wr[y] == 0);
  else if (wk[x] == 0)  // D
    return !((wr[x] > 1 && wr[y] == 0) || (wr[x] == 1 && wr[y] > 0));

  // None of those positions: the position is not mate or stalemate.
  return true;
}

// Implementation written and tested by Carlo Wood - 2025/07/20.
bool Board::determine_check() const
{
  // This function must also work for illegal positions where whites
  // rook can capture the black king. It does not have to work for
  // illegal positions where the two kings are next to each other:
  //
  // we check whether or not the white rook as a direct line of sight
  // to the black king, without being concerned about whose turn it is.

  // Set up coordinates for easy swapping.
  auto [x, y, board_size_x, board_size_y] = Square::default_coordinates();
  auto [bk, wk, wr] = abbreviations();

  // Determine if the rook is on the same file and/or row as the black king.
  bool same_file = bk[x] == wr[x];
  bool same_row  = bk[y] == wr[y];

  // If the both are true, then the rook was captured and there is no check.
  // If both are false, then the black king and white rook are not on one line
  // and it also isn't check.
  if (same_file == same_row)
    return false;

  // To simply the code below, lets flip the position if the king and rook are not on the same row.
  if (!same_row)
  {
    std::swap(x, y);
    std::swap(board_size_x, board_size_y);
  }

  // If the white king is not on the same row as the black king (and rook) then it isn't blocking the check.
  if (wk[y] != bk[y])
    return true;

  int min_x = std::min(bk[x], wr[x]);
  int max_x = std::max(bk[x], wr[x]);

  // If the white king is between the black king and the white rook, it isn't check.
  return !(min_x < wk[x] && wk[x] < max_x);
}

// Written and tested by Carlo Wood - 2025/07/18.
bool Board::determine_legal(Color to_move) const
{
  using namespace coordinates;
  auto [bk, wk, wr] = abbreviations();

  // Kings can't be next to eachother, or occupy the same square.
  // Also return false if they occupy the same square.
  if (std::abs(bk[x] - wk[x]) <= 1 && std::abs(bk[y] - wk[y]) <= 1)
    return false;

  // The white rook and white king can't occupy the same square.
  if (wr == wk)
    return false;

  // If the white rook and the black king occupy the same square,
  // black just took the rook if white is to play, otherwise the position is illegal.
  if (wr == bk)
    return to_move == white;            // Not illegal if white is to play.

  // The remaining positions are all legal if it is black to play.
  if (to_move == black)
    return true;

  // If it is white to play, then the position is illegal if black is in check.
  return !determine_check();
}

// Implementation written and tested by Carlo Wood - 2025/07/19.
Board::Mate Board::determine_mate(Color to_move) const
{
  // (Stale)mate can only occur when it is blacks turn to move.
  if (to_move != black)
    return Mate::no;

  // Now we know it's blacks turn, so we can use black_has_moves.
  // If black has (legal) moves then it is neither mate nor stalemate.
  if (black_has_moves())
    return Mate::no;

  // Black has no legal moves, therefore it must be mate or stalemate.
  return determine_check() ? Mate::yes : Mate::stalemate;
}

// Written and tested by Carlo Wood - 2025/07/20.
bool Board::determine_draw(Color to_move) const
{
  using namespace coordinates;

  // Set up coordinates for easy swapping.
  auto [x, y, board_size_x, board_size_y] = Square::default_coordinates();
  auto [bk, wk, wr] = abbreviations();

  if (to_move == white)
    // The position is only draw if the rook was captured.
    return bk == wr;

  // The position is a draw if it is stalemate.
  Mate mate = determine_mate(to_move);
  if (mate == Mate::stalemate)
    return true;

  // If the black king is in a corner of an edge and a virtual edge, then
  // it has two potential squares to step out of reach: R and K, see below.
  // The only way to stop the black king from going to K is if the white
  // king is opposite of the black king, and the only way to stop the black
  // king from going to R is if that is röntgen protected by the white rook:
  //
  //   A:  ┏━0━1━2━3   B:  ┏━0━1━2━3
  //     0 ┃ ♜   ·       0 ┃ ♜ ♜ ♜ ♔ R
  //     1 ┃ ♜ ·   ·     1 ┃   ·   · K
  //     2 ┃ ♜   ·       2 ┃ ·   · ♚
  //     3 ┃ ♔ · ♚ ·,    3 ┃   ·   ·
  //         R K

  // To simply the code below, lets flip the position if the king is on the right virtual edge.
  if (bk[x] == board_size_x - 1)
  {
    std::swap(x, y);
    std::swap(board_size_x, board_size_y);
  }

  // Now the king is on the bottom virtual edge, or at no virtual edge at all.
  // If it is not at a virtual edge, it is not a draw.
  if (bk[y] != board_size_y - 1)
    return false;

  // Now the black king is on the bottom virtual edge:
  //   A:  ┏━0━1━2━3
  //     0 ┃ ·   ·
  //     1 ┃   ·   ·
  //     2 ┃ ·   ·
  //     3 ┃ ♔ ♔ ♔ ♔

  // Hence it is a draw unless the position is position A.
  return !(bk[x] == 0 && wk[x] == 2 && wk[y] == board_size_y - 1 && wr[x] == 0);
}

#if 0
std::vector<Board> Board::get_succeeding_boards(Color to_move) const
{
  //Dout(dc::notice, "Board::get_succeeding_boards(" << to_move << ")");

  std::vector<Board> result;

  auto [bk, wk, wr] = abbreviations();
  if (wr == bk)
  {
    // With white to move, do not generate any moves once the rook is captured.
  }
  else if (to_move == black)
  {
    using namespace coordinates;
    Board succeeding_board(*this);
    int const cbkx = bk[x];
    int const cbky = bk[y];
    ASSERT(0 <= cbkx && cbkx < board_size_x);
    ASSERT(0 <= cbky && cbky < board_size_y);
    int const dx_min = std::max(-1, -cbkx);
    int const dx_max = std::min(1U, Size::board::x - 1 - cbkx);
    int const dy_min = std::max(-1, -cbky);
    int const dy_max = std::min(1U, Size::board::y - 1 - cbky);
    for (int dx = dx_min; dx <= dx_max; ++dx)
    {
      for (int dy = dy_min; dy <= dy_max; ++dy)
      {
        if (dx == 0 && dy == 0)
          continue;
        //Dout(dc::notice, "Before: " << succeeding_board);
        //Dout(dc::notice, "Calling set_black_king_square(" << cbkx << " + " << dx << " = " << (cbkx + dx) << ", " <<
        //    cbky << " + " << dy << " = " << (cbky + dy) << ")");
        succeeding_board.set_black_king_square({cbkx + dx, cbky + dy});
        //Dout(dc::notice, "  result: " << succeeding_board);
        result.push_back(succeeding_board);
      }
    }
  }
  else
  {
    using namespace coordinates;
    int const cwkx = wk[x];
    int const cwky = wk[y];
    ASSERT(0 <= cwkx && cwkx < board_size_x);
    ASSERT(0 <= cwky && cwky < board_size_y);
    int const dx_min = std::max(-1, -cwkx);
    int const dx_max = std::min(1U, Size::board::x - 1 - cwkx);
    int const dy_min = std::max(-1, -cwky);
    int const dy_max = std::min(1U, Size::board::y - 1 - cwky);
    {
      Board succeeding_board(*this);
      for (int dx = dx_min; dx <= dx_max; ++dx)
      {
        for (int dy = dy_min; dy <= dy_max; ++dy)
        {
          if (dx == 0 && dy == 0)
            continue;
          succeeding_board.set_white_king_square({cwkx + dx, cwky + dy});
          result.push_back(succeeding_board);
        }
      }
    }
    {
      Board succeeding_board(*this);
      int const cwrx = wr[x];
      int const cwry = wr[y];
      ASSERT(0 <= cwrx && cwrx < board_size_x);
      ASSERT(0 <= cwry && cwry < board_size_y);
      for (int horvert = 0; horvert < 2; ++horvert)
      {
        int board_size = horvert == 0 ? board_size_x : board_size_y;
        for (int dir = -1; dir <= 1; dir += 2)
        {
          int end = dir == -1 ? -1 : board_size;
          for (int dist = 1; dist < std::abs(end - (horvert == 0 ? cwrx : cwry)); ++dist)
          {
            int wrx = cwrx;
            int wry = cwry;
            if (horvert == 0)   // Move horizontal.
              wrx += dir * dist;
            else                // Move vertical.
              wry += dir * dist;
            // Can't move through the white king.
            if (wrx == cwkx && wry == cwky)
              break;
            succeeding_board.set_white_rook_square({wrx, wry});
            result.push_back(succeeding_board);
          }
        }
      }
    }
  }

  return result;
}
#endif

//=============================================================================
// Printing a board.
//

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
std::u8string_view const left_side  = u8" ┃";
std::u8string_view const bcorner    = u8" ┗";
std::u8string_view const top_side   = u8"━";
std::u8string_view const marker     = u8" X";
} // namespace

// Write std::u8string_view to an ostream as-is.
void raw_utf8(std::ostream& os, std::u8string_view const& sv)
{
  os.write(reinterpret_cast<const char*>(sv.data()), sv.size());
}

void print_none_to(std::ostream& os, Color color)
{
  std::u8string_view const& sv = color == Black ? black_none : white_none;
  os.write(reinterpret_cast<const char*>(sv.data()), sv.size());
}

void print_king_to(std::ostream& os, Color color)
{
  std::u8string_view const& sv = color == Black ? black_king : white_king;
  os.write(reinterpret_cast<const char*>(sv.data()), sv.size());
}

void print_rook_to(std::ostream& os)
{
  std::u8string_view const& sv = white == Black ? black_rook : white_rook;
  os.write(reinterpret_cast<const char*>(sv.data()), sv.size());
}

void print_marker_to(std::ostream& os)
{
  std::u8string_view const& sv = marker;
  os.write(reinterpret_cast<const char*>(sv.data()), sv.size());
}

//static
void Board::utf8art(std::ostream& os, Color to_move, bool xyz, std::function<Figure (Square)> select_figure)
{
  // Print top to bottom.
  bool skip_top = true;
  for (int y = Size::board::y - 1; y >= 0; --y)
  {
    bool restart = false;
    for (;;)    // Allow restarting this y value when the first piece is encountered.
    {
      if (!skip_top)
      {
        os << std::setw(2) << std::setfill(' ') << std::right << (y + 1);
        os.write(reinterpret_cast<const char*>(left_side.data()), left_side.size());
      }
      // Print left to right.
      for (int x = 0; x < Size::board::x; ++x)
      {
        Figure figure = select_figure({x, y});
        if (skip_top)
        {
          if (figure == Figure::none)
            continue;
          skip_top = false;
          restart = true;
          break;
        }
        if (figure == Figure::black_king)
          print_king_to(os, black);
        else if (figure == Figure::white_king)
          print_king_to(os, white);
        else if (figure == Figure::white_rook)
          print_rook_to(os);
        else if (figure == Figure::marker)
          print_marker_to(os);
        else
          print_none_to(os, (x + y) % 2 == 1 ? black : white);
      }
      if (restart)
      {
        restart = false;
        continue;
      }
      if (!skip_top)
        os << "\n";
      break;
    }
  }
  if (xyz)
  {
    int line_total = 2 * Size::board::x - 15; // 15 = length("━a━b━c···━x━y━z").
    std::cout << "   ┗━a━b━c";
    int line_left = line_total / 2;
    int line_right = line_total - line_left;
    for (int i = 0; i < line_left; ++i)
      std::cout << "━";
    std::cout << "···";
    for (int i = 0; i < line_right; ++i)
      std::cout << "━";
    std::cout << "━x━y━z\n";
    std::cout << "     🢐";
    for (int i = 0; i < line_left + 3; ++i)
      std::cout << "─";
    std::cout << "n";
    for (int i = 0; i < line_right + 2; ++i)
      std::cout << "─";
    std::cout << "🢒 " << to_move << " to move\n";
  }
  else
  {
    os << "  ";
    os.write(reinterpret_cast<const char*>(bcorner.data()), bcorner.size());
    for (int x = 0; x < Size::board::x; ++x)
    {
      os.write(reinterpret_cast<const char*>(top_side.data()), top_side.size());
      os << static_cast<char>('a' + x % 26);
    }
    os << '\n';
    if (Size::board::x > 26)
    {
      os << "\n    ";
      for (int x = 0; x < Size::board::x; ++x)
      {
        int z = x / 26;
        os << ' ';
        if (z == 0)
          os << ' ';
        else
          os << static_cast<char>('a' + z % 26);
      }
      os << "\n";
    }
  }
}

void Board::utf8art(std::ostream& os, Color to_move, bool xyz, Square marker) const
{
  auto [bk, wk, wr] = abbreviations();
  Board::utf8art(os, to_move, xyz, [bk, wk, wr, marker, this](Square pos){
    if (pos == bk)
      return Figure::black_king;
    else if (pos == wk)
      return Figure::white_king;
    else if (pos == wr)
      return Figure::white_rook;
    else if (pos == marker)
      return Figure::marker;
    else
      return Figure::none;
  });
}

#if 0
std::string Board::get_move(Board const& to_board)
{
  size_t diff = as_index() ^ to_board.as_index();

  int x, y;
  char piece;

  // Check which piece moved by examining the XOR mask
  if ((diff & Board::black_king_mask))
  {
    SquareCompact to_black_king = to_board.black_king();
    x = Board::x_coord(to_black_king);
    y = Board::y_coord(to_black_king);
    piece = 'K';
  }
  else if ((diff & Board::white_king_mask))
  {
    SquareCompact to_white_king = to_board.white_king();
    x = Board::x_coord(to_white_king);
    y = Board::y_coord(to_white_king);
    piece = 'K';
  }
  else
  {
    // There must have been a move.
    ASSERT((diff & Board::white_rook_mask));
    SquareCompact to_white_rook = to_board.white_rook();
    x = Board::x_coord(to_white_rook);
    y = Board::y_coord(to_white_rook);
    piece = 'R';
  }

  std::ostringstream oss;
  oss << piece << static_cast<char>('a' + x) << (y + 1);
  return oss.str();
}
#endif

#ifdef CWDEBUG
void Board::debug_utf8art(libcwd::channel_ct const& debug_channel, Square marker) const
{
  if (!debug_channel.is_on())
    return;

  Dout(debug_channel, *this << ":");
  if (Size::board::x > 10)
  {
    Dout(debug_channel|continued_cf, "    ");
    for (int x = 0; x < Size::board::x; ++x)
    {
      int tenth = x / 10;
      Dout(dc::continued, ' ');
      if (tenth == 0)
        Dout(dc::continued, ' ');
      else
        Dout(dc::continued, (tenth % 10));
    }
    Dout(dc::finish, "");
  }
  Dout(debug_channel|continued_cf, "  " << utils::print_using(corner, &raw_utf8));
  for (int x = 0; x < Size::board::x; ++x)
    Dout(dc::continued, utils::print_using(top_side, &raw_utf8) << (x % 10));
  Dout(dc::finish, "");
  // Print top to bottom.
  auto [bk, wk, wr] = abbreviations();
  for (int y = 0; y < Size::board::y; ++y)
  {
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill(' ') << std::right << (y % 100) << utils::print_using(left_side, &raw_utf8);
    // Print left to right.
    for (int x = 0; x < Size::board::x; ++x)
    {
      Square square_coordinates(x, y);

      if (square_coordinates == bk)
        print_king_to(oss, black);
      else if (square_coordinates == wk)
        print_king_to(oss, white);
      else if (square_coordinates == wr)
        print_rook_to(oss);
      else if (square_coordinates == marker)
        print_marker_to(oss);
      else
        print_none_to(oss, (x + y) % 2 == 1 ? black : white);
    }
    Dout(debug_channel, oss.str());
  }
}

std::ostream& operator<<(std::ostream& os, Board::Mate mate)
{
  switch (mate)
  {
    case Board::Mate::yes:
      os << "mate";
      break;
    case Board::Mate::stalemate:
      os << "stalemate";
      break;
    case Board::Mate::no:
      os << "not (stale)mate";
      break;
  }
  return os;
}

void Board::print_on(std::ostream& os) const
{
  os <<
    "{black king:" << Square{black_king()} <<
   ", white king:" << Square{white_king()} <<
   ", white rook:" << Square{white_rook()} << '}';
}
#endif
