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
  encoded_type encoded_be = utils::hton(encoded_);
  char const* char_ptr = reinterpret_cast<char const*>(&encoded_be);
  os.write(char_ptr, sizeof(encoded_be));
}

void Classification::read_from(std::istream& is)
{
  //FIXME: this is for the old file format.
  /*Classification::ply_type*/ uint16_t mate_in_ply_be;
  char* char_ptr = reinterpret_cast<char*>(&mate_in_ply_be);
  is.read(char_ptr, sizeof(mate_in_ply_be));
  encoded_type mate_in_ply  = utils::ntoh(mate_in_ply_be);
  if (mate_in_ply_be == 0xffff)
    mate_in_ply = unknown_ply;

  uint8_t bits_be;
  char_ptr = reinterpret_cast<char*>(&bits_be);
  is.read(char_ptr, sizeof(bits_be));
  encoded_type bits = utils::ntoh(bits_be);

  encoded_ = (mate_in_ply << mate_in_ply_shift) | bits;
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
  ply_type const mate_in_ply = static_cast<ply_type>(encoded_ >> mate_in_ply_shift);
  if (mate_in_ply == unknown_ply)
    os << "<unknown>";
  else
    os << static_cast<uint32_t>(mate_in_ply);
  os << '}';
}
#endif
