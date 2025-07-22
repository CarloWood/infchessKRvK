#include "sys.h"
#include "Position.h"
#include <vector>
#include <iostream>
#include <cassert>
#include "debug.h"

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  constexpr int board_size = 5;

  int total_positions = 0;
  int draw_positions = 0;
  int black_in_check_positions = 0;
  int mate_positions = 0;
  int stalemate_positions = 0;

  // Generate all possible positions
  std::vector<Position> positions = Position::analyze_all(board_size);
  total_positions = positions.size();

  for (Position const& pos : positions)
  {
    if (pos.is_draw())
      ++draw_positions;
    if (pos.is_check())
      ++black_in_check_positions;
    if (pos.is_mate())
      ++mate_positions;
    if (pos.is_stalemate())
      ++stalemate_positions;
  }

  std::cout << "Total legal positions: " << total_positions << std::endl;
  std::cout << "Draw positions: " << draw_positions << std::endl;
  std::cout << "Black in check positions: " << black_in_check_positions << std::endl;
  std::cout << "Mate positions: " << mate_positions << std::endl;
  std::cout << "Stalemate positions: " << stalemate_positions << std::endl;
}
