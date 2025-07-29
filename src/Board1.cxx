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

namespace version1 {

// Implementation written and tested by Carlo Wood - 2025/07/19.
bool Board::black_has_moves() const
{
  // This function assumes it is black to move.
  // Therefore it should only be called when black is to move.
  ASSERT(black_is_to_move());

  // Set up coordinates for easy swapping.
  auto [x, y] = Square::default_coordinates();
  auto [bk, wk, wr] = Board::abbreviations();

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
    std::swap(x, y);

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
  auto [x, y] = Square::default_coordinates();
  auto [bk, wk, wr] = Board::abbreviations();

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
    std::swap(x, y);

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

  // Kings can't be next to eachother, or occupy the same square.
  if (kings_next_to_eachother())
    return false;

  // The white rook and white king can't occupy the same square.
  if (white_rook() == white_king())
    return false;

  // If the white rook and the black king occupy the same square,
  // black just took the rook if white is to play, otherwise the position is illegal.
  if (white_rook() == black_king())
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

  if (to_move == white)
    // The position is only draw if the rook was captured.
    return black_king() == white_rook();

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

  // Set up coordinates for easy swapping.
  auto [x, y] = Square::default_coordinates();
  auto [bk, wk, wr] = Board::abbreviations();

  // To simply the code below, lets flip the position if the king is on the right virtual edge.
  if (bk[x] == Board::board_size - 1)
    std::swap(x, y);

  // Now the king is on the bottom virtual edge, or at no virtual edge at all.
  // If it is not at a virtual edge, it is not a draw.
  if (bk[y] != Board::board_size - 1)
    return false;

  // Now the black king is on the bottom virtual edge:
  //   A:  ┏━0━1━2━3
  //     0 ┃ ·   ·
  //     1 ┃   ·   ·
  //     2 ┃ ·   ·
  //     3 ┃ ♔ ♔ ♔ ♔

  // Hence it is a draw unless the position is position A.
  return !(bk[x] == 0 && wk[x] == 2 && wk[y] == Board::board_size - 1 && wr[x] == 0);
}

std::vector<Board> Board::get_succeeding_boards(Color to_move) const
{
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
    int const dx_min = std::max(-1, -cbkx);
    int const dx_max = std::min(1U, Board::board_size - 1 - cbkx);
    int const dy_min = std::max(-1, -cbky);
    int const dy_max = std::min(1U, Board::board_size - 1 - cbky);
    for (int dx = dx_min; dx <= dx_max; ++dx)
    {
      for (int dy = dy_min; dy <= dy_max; ++dy)
      {
        if (dx == 0 && dy == 0)
          continue;
        succeeding_board.set_black_king_square({cbkx + dx, cbky + dy});
        result.push_back(succeeding_board);
      }
    }
  }
  else
  {
    using namespace coordinates;
    int const cwkx = wk[x];
    int const cwky = wk[y];
    int const dx_min = std::max(-1, -cwkx);
    int const dx_max = std::min(1U, Board::board_size - 1 - cwkx);
    int const dy_min = std::max(-1, -cwky);
    int const dy_max = std::min(1U, Board::board_size - 1 - cwky);
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
      for (int horvert = 0; horvert < 2; ++horvert)
      {
        for (int dir = -1; dir <= 1; dir += 2)
        {
          int end = dir == -1 ? -1 : Board::board_size;
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

//static
void Board::utf8art(std::ostream& os, std::function<Figure (Square)> select_figure)
{
  // Print top to bottom.
  for (int y = Board::board_size - 1; y >= 0; --y)
  {
    os << std::setw(2) << std::setfill(' ') << std::right << (y + 1);
    os.write(reinterpret_cast<const char*>(left_side.data()), left_side.size());
    // Print left to right.
    for (int x = 0; x < Board::board_size; ++x)
    {
      Figure figure = select_figure({x, y});
      if (figure == Figure::black_king)
        print_king_to(os, black);
      else if (figure == Figure::white_king)
        print_king_to(os, white);
      else if (figure == Figure::white_rook)
        print_rook_to(os);
      else
        print_none_to(os, (x + y) % 2 == 1 ? black : white);
    }
    os << "\n";
  }
  os << "  ";
  os.write(reinterpret_cast<const char*>(bcorner.data()), bcorner.size());
  for (int x = 0; x < Board::board_size; ++x)
  {
    os.write(reinterpret_cast<const char*>(top_side.data()), top_side.size());
    os << static_cast<char>('a' + x % 26);
  }
  os << '\n';
  if (Board::board_size > 26)
  {
    os << "\n    ";
    for (int x = 0; x < Board::board_size; ++x)
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

void Board::utf8art(std::ostream& os) const
{
  Board::utf8art(os, [this](Square pos){
    square_coordinates_type sc = Board::to_square_coordinates(pos);
    if (black_king() == sc)
      return Figure::black_king;
    else if (white_king() == sc)
      return Figure::white_king;
    else if (white_rook() == sc)
      return Figure::white_rook;
    else
      return Figure::none;
  });
}

#ifdef CWDEBUG
void Board::debug_utf8art(libcwd::channel_ct const& debug_channel) const
{
  if (!debug_channel.is_on())
    return;

  Dout(debug_channel, *this << ":");
  if (Board::board_size > 10)
  {
    Dout(debug_channel|continued_cf, "    ");
    for (int x = 0; x < Board::board_size; ++x)
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
  for (int x = 0; x < Board::board_size; ++x)
    Dout(dc::continued, utils::print_using(top_side, &raw_utf8) << (x % 10));
  Dout(dc::finish, "");
  // Print top to bottom.
  for (int y = 0; y < Board::board_size; ++y)
  {
    std::ostringstream oss;
    oss << std::setw(2) << std::setfill(' ') << std::right << (y % 100) << utils::print_using(left_side, &raw_utf8);
    // Print left to right.
    for (int x = 0; x < Board::board_size; ++x)
    {
      Square pos{x, y};
      square_coordinates_type square_coordinates = Board::to_square_coordinates(pos);

      if (black_king() == square_coordinates)
        print_king_to(oss, black);
      else if (white_king() == square_coordinates)
        print_king_to(oss, white);
      else if (white_rook() == square_coordinates)
        print_rook_to(oss);
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
    "{black king:" << to_square(black_king()) <<
   ", white king:" << to_square(white_king()) <<
   ", white rook:" << to_square(white_rook()) << '}';
}
#endif

} // namespace version1
