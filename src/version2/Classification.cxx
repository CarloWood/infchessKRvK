#include "sys.h"
#include "Classification.h"
#include "Board.h"
#include "../coordinates.h"
#include "utils/endian.h"
#include <iostream>

void Classification::determine(Board const& board, Color to_move)
{
  reset();
  if (board.determine_check())
    set_check();
  if (board.determine_draw(to_move))
    set_draw();
  // Position can not be (stale)mate unless it is black to move.
  if (to_move == black)
  {
    bool has_moves = board.black_has_moves();
    Board::Mate mate = board.determine_mate(black);
    if (mate == Board::Mate::yes)
    {
      ASSERT(!has_moves && is_check() && !is_draw());
      set_mate();
    }
    else if (mate == Board::Mate::stalemate)
    {
      ASSERT(!has_moves && !is_check() && is_draw());
      set_stalemate();
    }
    else
    {
      using namespace coordinates;
      // If it is blacks turn and it is not mate, nor stalemate, then black
      // must have (legal) moves - unless the king is at a virtual edge,
      // because stepping off the board is not being counted as a legal move.
      ASSERT(has_moves ||
          Board::x_coord(board.black_king()) == Size::board::x - 1 ||
          Board::y_coord(board.black_king()) == Size::board::y - 1);
    }
  }
  // Mark that board as legal.
  set_legal();
}

#ifdef CWDEBUG
//static
std::string Classification::state_str(encoded_type state)
{
  if (!(state & legal))
    return "illegal";
  if ((state & mate))
    return "mate";
  if ((state & stalemate))
    return "stalemate";
  std::string str = "normal";
  if ((state & check))
    str = "check";
  if ((state & draw))
  {
    if (str == "normal")
      str = "draw";
    else
      str += "|draw";
  }
  return str;
}

void Classification::print_on(std::ostream& os) const
{
  os << '{';
  auto const bits = encoded_ & bits_mask;
  os << "bits:" << static_cast<uint32_t>(bits);
  if ((bits & legal))
    os << " (" << state_str(bits) << ")";
  os << ", mate_in_ply:";
  ply_type const encoded_mate_in_ply = static_cast<ply_type>(encoded_ >> mate_in_ply_shift);
  if (encoded_mate_in_ply == encoded_unknown_ply)
    os << "<unknown>";
  else
    os << static_cast<uint32_t>(encoded_mate_in_ply - 1);       // Subtract one to get the real number of ply.
  os << '}';
}
#endif
