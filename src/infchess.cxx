#include "sys.h"
#include "Board.h"
#include "Data.h"
#include "utils/print_using.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <map>
#include "debug.h"

struct DistanceCompare
{
  bool operator()(Board const& lhs, Board const& rhs) const
  {
    return lhs.distance_less(rhs);
  }
};

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  using namespace std::chrono_literals; // For ms.

#if 0
  Board current_position({0, 1}, {2, 1}, {3, 5}, white);
  Dout(dc::notice, "Current position:" << utils::print_using(current_position, &Board::utf8art));

  std::vector<Board> positions1 = current_position.preceding_positions();
  for (Board const& board : positions1)
    Dout(dc::notice, utils::print_using(board, &Board::utf8art));

  return 0;
#endif

  std::vector<Board> positions;

  // Generate all positions that are already mate.
  for (int wrc = 2; wrc < Board::horizontal_limit; ++wrc)
  {
    positions.emplace_back(Square{0, 0}, Square{0, 2}, Square{wrc, 0}, black);
    positions.emplace_back(Square{0, 0}, Square{1, 2}, Square{wrc, 0}, black);
    positions.emplace_back(Square{0, 0}, Square{2, 0}, Square{0, wrc}, black);
    positions.emplace_back(Square{0, 0}, Square{2, 1}, Square{0, wrc}, black);
  }
  for (int kn = 1; kn < Board::horizontal_limit; ++kn)
  {
    for (int wrn = 0; wrn < Board::horizontal_limit; ++wrn)
      if (std::abs(wrn  - kn) > 1)
      {
        positions.emplace_back(Square{kn, 0}, Square{kn, 2}, Square{wrn, 0}, black);
        positions.emplace_back(Square{0, kn}, Square{2, kn}, Square{0, wrn}, black);        // Flipped
      }
  }

  std::map<Board, Data, DistanceCompare> data(DistanceCompare{});

  // Store all positions that are already mate.
  for (Board const& board : positions)
    data.try_emplace(board, 0);

  for (auto const& value : data)
  {
    Board const& current_position(value.first);
    Dout(dc::notice, "Mate position: " << utils::print_using(current_position, &Board::utf8art));

    positions = current_position.preceding_positions();
    for (Board const& board : positions)
    {
      Dout(dc::notice, "Mate in one position:" << utils::print_using(board, &Board::utf8art));

      std::vector<Board> positions2 = board.preceding_positions();
      for (Board const& board2 : positions2)
      {
        Dout(dc::notice, "Mate in two ply position:" << utils::print_using(board2, &Board::utf8art));
      }
    }
  }
}
