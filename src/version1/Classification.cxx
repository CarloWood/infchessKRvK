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
      ASSERT(has_moves || Board::x_coord(board.black_king()) == Board::board_size_x - 1 ||
          Board::y_coord(board.black_king()) == Board::board_size_y - 1);
    }
  }
  // Mark that board as legal.
  set_legal();
}

void Classification::write_to(std::ostream& os) const
{
  using ply_type = uint8_t;
  ASSERT(mate_in_moves_ == -1 || (0 <= mate_in_moves_ && mate_in_moves_ < 256));
  ply_type mate_in_moves = mate_in_moves_ & 0xff;
  auto mate_in_moves_be = utils::hton(mate_in_moves);
  char const* char_ptr = reinterpret_cast<char const*>(&mate_in_moves_be);
  os.write(char_ptr, sizeof(mate_in_moves_be));
  using bits_type = uint8_t;
  bits_type bits_be = utils::hton(bits_);
  char_ptr = reinterpret_cast<char const*>(&bits_be);
  os.write(char_ptr, sizeof(bits_type));
}

void Classification::read_from(std::istream& is)
{
  using ply_type = uint8_t;
  uint8_t mate_in_moves_be;
  char* char_ptr = reinterpret_cast<char*>(&mate_in_moves_be);
  is.read(char_ptr, sizeof(mate_in_moves_be));
  mate_in_moves_ = mate_in_moves_be == 0xff ? -1 : static_cast<int>(utils::ntoh(mate_in_moves_be));
  using bits_type = uint8_t;
  bits_type bits_be;
  char_ptr = reinterpret_cast<char*>(&bits_be);
  is.read(char_ptr, sizeof(bits_type));
  bits_ = utils::ntoh(bits_be);
}

#ifdef CWDEBUG
void Classification::print_on(std::ostream& os) const
{
  os << "{bits:" << static_cast<int>(bits_) << ", mate_in_moves:" << mate_in_moves_ << '}';
}
#endif
