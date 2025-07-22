#include "sys.h"
#include "Position.h"
#include "Graph.h"
#include "TwoKings.h"
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

  // Construct the initial graph with all positions that are already mate.
  Graph graph(board_size);

  // Generate all positions that are mate in 1, 2, ..., `ply` moves.
  int max_ply = 21;
  Debug(libcwd::libcw_do.off());
  graph.generate(max_ply);
  Debug(libcwd::libcw_do.on());

  // All squares that a rook can stand on, given the squares of the kings and the number of moves.
  using rook_positions_type = std::vector<Square>;
  // A map with rook positions as function of kings positions, given the number of moves.
  using kings_to_rooks_map_type = std::map<TwoKings, rook_positions_type>;
  // A vector with a kings_to_rooks_type maps for each given number of moves.
  std::vector<kings_to_rooks_map_type> ply_to_map(max_ply + 1);

  // For a given number of moves that a position must be mate...
  for (int ply = 0; ply < ply_to_map.size(); ++ply)
  {
    kings_to_rooks_map_type& kings_to_rooks_map = ply_to_map[ply];      // The map to use.
    // ...run over all positions that are mate in ply moves.
    for (Graph::nodes_type::const_iterator const& mate_in_ply_position : graph.mate_in_ply(ply))
    {
      // Create key containing the kings positions.
      kings_to_rooks_map_type::key_type key{
        mate_in_ply_position->first.black_king(),
        mate_in_ply_position->first.white_king()
      };
      // Get the relevant rook_positions_type.
      rook_positions_type& rook_positions = kings_to_rooks_map[key];
      // Add the square that the rook is on.
      rook_positions.push_back(mate_in_ply_position->first.white_rook());
    }

    for (kings_to_rooks_map_type::value_type& value : kings_to_rooks_map)
    {
      TwoKings const& two_kings = value.first;
      rook_positions_type const& rook_positions = value.second;

      Board::utf8art(std::cout, [&](Square pos) -> Board::Figure {
        if (pos == two_kings.bk())
          return Board::Figure::black_king;
        else if (pos == two_kings.wk())
          return Board::Figure::white_king;
        else
        {
          for (int i = 0; i < rook_positions.size(); ++i)
            if (pos == rook_positions[i])
              return Board::Figure::white_rook;
        }
        return Board::Figure::none;
      });
      std::cout << "Mate in " << ply << " ply";
      if (ply % 2 == 0)
        std::cout << " (black to move)";
      std::cout << std::endl;
    }
  }
}
