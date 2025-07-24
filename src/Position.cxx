#include "sys.h"
#include "Position.h"
#include <vector>
#include <cstdlib>
#include <iostream>
#include "debug.h"

// Implementation written and tested by Carlo Wood - 2025/07/19.
Position::Mate Position::determine_mate() const
{
  // (Stale)mate can only occur when it is blacks turn to move.
  if (to_move_ != black)
    return Mate::no;

  // Now we know it's blacks turn, so we can use black_has_moves.
  // If black has (legal) moves then it is neither mate nor stalemate.
  if (black_has_moves())
    return Mate::no;

  // Black has no legal moves, therefore it must be mate or stalemate.
  return determine_check() ? Mate::yes : Mate::stalemate;
}

// Written and tested by Carlo Wood - 2025/07/20.
bool Position::determine_draw() const
{
  using namespace coordinates;

  if (to_move_ == white)
    // The position is only draw if the rook was captured.
    return black_king_ == white_rook_;

  // The position is a draw if it is stalemate.
  Mate mate = determine_mate();
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
  if (bk[x] == board_size_ - 1)
    std::swap(x, y);

  // Now the king is on the bottom virtual edge, or at no virtual edge at all.
  // If it is not at a virtual edge, it is not a draw.
  if (bk[y] != board_size_ - 1)
    return false;

  // Now the black king is on the bottom virtual edge:
  //   A:  ┏━0━1━2━3
  //     0 ┃ ·   ·
  //     1 ┃   ·   ·
  //     2 ┃ ·   ·
  //     3 ┃ ♔ ♔ ♔ ♔

  // Hence it is a draw unless the position is position A.
  return !(bk[x] == 0 && wk[x] == 2 && wk[y] == board_size_ - 1 && wr[x] == 0);
}

void Position::classify()
{
  classification_.reset();
  if (determine_check())
    classification_.set_check();
  if (determine_draw())
    classification_.set_draw();
  // Position can not be (stale)mate unless it is black to move.
  if (to_move_ == black)
  {
    bool has_moves = black_has_moves();
    Mate mate = determine_mate();
    if (mate == Mate::yes)
    {
      ASSERT(!has_moves && classification_.is_check() && !classification_.is_draw());
      classification_.set_mate();
    }
    else if (mate == Mate::stalemate)
    {
      ASSERT(!has_moves && !classification_.is_check() && classification_.is_draw());
      classification_.set_stalemate();
    }
    else
    {
      ASSERT(has_moves || black_king_[0] == board_size_ - 1 || black_king_[1] == board_size_ - 1);
    }
  }
}

std::ostream& operator<<(std::ostream& os, Position::Mate mate)
{
  switch (mate)
  {
    case Position::Mate::yes:
      os << "mate";
      break;
    case Position::Mate::stalemate:
      os << "stalemate";
      break;
    case Position::Mate::no:
      os << "not (stale)mate";
      break;
  }
  return os;
}

#ifdef CWDEBUG
void Position::print_on(std::ostream& os) const
{
  os << static_cast<Board const&>(*this) << ", to move: " << to_move_;
}
#endif
