#include "sys.h"
#include "Classification.h"
#include "Board.h"
#include <iostream>

namespace version0 {

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
      ASSERT(has_moves || board.black_king()[x] == board.board_size() - 1 || board.black_king()[y] == board.board_size() - 1);
    }
  }
}

#ifdef CWDEBUG
void Classification::print_on(std::ostream& os) const
{
  os << "{bits:" << static_cast<int>(bits_) << ", mate_in_moves:" << mate_in_moves_ << '}';
}
#endif

} // namespace version0

namespace version1 {

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
      ASSERT(has_moves || board.black_king()[x] == board.board_size() - 1 || board.black_king()[y] == board.board_size() - 1);
    }
  }
}

#ifdef CWDEBUG
void Classification::print_on(std::ostream& os) const
{
  os << "{bits:" << static_cast<int>(bits_) << ", mate_in_moves:" << mate_in_moves_ << '}';
}
#endif

} // namespace version1
