#include "sys.h"
#include "Classification.h"
#include "Board.h"
#include "coordinates.h"
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

void Classification::write_to(std::ostream& os) const
{
  auto mate_in_moves_be = utils::hton(mate_in_moves_);
  char const* char_ptr = reinterpret_cast<char const*>(&mate_in_moves_be);
  os.write(char_ptr, sizeof(mate_in_moves_be));

  bits_type bits_be = utils::hton(bits_);
  char_ptr = reinterpret_cast<char const*>(&bits_be);
  os.write(char_ptr, sizeof(bits_type));
}

void Classification::read_from(std::istream& is)
{
  //FIXME: this is for the old file format.
  /*Classification::ply_type*/ uint16_t mate_in_moves_be;
  char* char_ptr = reinterpret_cast<char*>(&mate_in_moves_be);
  is.read(char_ptr, sizeof(mate_in_moves_be));
  mate_in_moves_  = utils::ntoh(mate_in_moves_be);
  if (mate_in_moves_be == 0xffff)
    mate_in_moves_ = unknown_ply;

  bits_type bits_be;
  char_ptr = reinterpret_cast<char*>(&bits_be);
  is.read(char_ptr, sizeof(bits_type));
  bits_ = utils::ntoh(bits_be);
}

#ifdef CWDEBUG
//static
std::string Classification::state_str(uint8_t state)
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
  os << "bits:" << state_str(bits_) <<
      ", mate_in_moves:" << mate_in_moves_;
  os << '}';
}
#endif
