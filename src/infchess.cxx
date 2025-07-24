#include "sys.h"
#include "Position.h"
#include "Graph.h"
#include "TwoKings.h"
#include "Box.h"
#include <vector>
#include <iostream>
#include <cassert>
#include "debug.h"

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  constexpr int board_size = 5;

  // Construct the initial graph with all positions that are already mate.
  Graph graph(board_size);

  int total_positions;
  int draw_positions = 0;
  int black_in_check_positions = 0;
  int mate_positions = 0;
  int stalemate_positions = 0;

  // Generate all possible positions
  graph.classify();

  std::vector<Graph::black_to_move_nodes_type::iterator> already_mate;

  // Black to move positions.
  {
    Graph::black_to_move_nodes_type& black_to_move_map = graph.black_to_move_map();
    total_positions = black_to_move_map.size();
    for (Graph::black_to_move_nodes_type::iterator current_position = black_to_move_map.begin();
        current_position != black_to_move_map.end(); ++current_position)
    {
      Board const& current_board = current_position->first;
      BlackToMoveData const& data = current_position->second;
      Classification const& pc = data;
      if (pc.is_draw())
        ++draw_positions;
      if (pc.is_check())
        ++black_in_check_positions;
      if (pc.is_mate())
      {
        ++mate_positions;
        already_mate.push_back(current_position);
      }
      if (pc.is_stalemate())
        ++stalemate_positions;
    }
  }
  // White to move positions.
  {
    Graph::white_to_move_nodes_type& white_to_move_map = graph.white_to_move_map();
    total_positions += white_to_move_map.size();
    for (Graph::white_to_move_nodes_type::iterator current_position = white_to_move_map.begin();
        current_position != white_to_move_map.end(); ++current_position)
    {
      Board const& current_board = current_position->first;
      WhiteToMoveData const& data = current_position->second;
      Classification const& pc = data;
      if (pc.is_draw())
        ++draw_positions;
    }
  }

  std::cout << "Total legal positions: " << total_positions << std::endl;
  std::cout << "Draw positions: " << draw_positions << std::endl;
  std::cout << "Black in check positions: " << black_in_check_positions << std::endl;
  std::cout << "Mate positions: " << mate_positions << std::endl;
  std::cout << "Stalemate positions: " << stalemate_positions << std::endl;

  // Generate links for all legal moves.
  graph.generate_edges();

  // Run over all positions that are already mate (as per the classification)
  // and mark all position that can reach those as mate in 1 ply.
  for (Graph::black_to_move_nodes_type::iterator iter : already_mate)
  {
    BlackToMoveData& data = iter->second;
    data.set_mate_in_ply(0);
    data.set_maximum_ply_on_parents(1);
  }

  // Print all positions that are mate in one ply.
  for (Graph::white_to_move_nodes_type::value_type board_data : graph.white_to_move_map())
  {
    if (board_data.second.ply() == 1)
      board_data.first.utf8art(std::cout);
  }

#if 0
  Graph::white_to_move_nodes_type::const_iterator iter = graph.white_to_move_map().begin();
  while (iter->first.black_king() == iter->first.white_rook())
    ++iter;
  Board const* board = &iter->first;
  for (;;)
  {
    std::cout << "\nCurrent position:\n";
    board->utf8art(std::cout);

    std::vector<Graph::nodes_type::const_iterator> child_positions = iter->second.child_positions();
    std::vector<Box> boxes;
    for (int i = 0; i < child_positions.size(); ++i)
    {
      boxes.emplace_back();
      boxes.back().stream() << "\n" << i << ":\n";
      child_positions[i]->first.utf8art(boxes.back().stream());
    }

    BoxRow row;
    size_t const MAX_WIDTH = 120;
    size_t current_width = 0;
    int i = 0;
    while (i < boxes.size())
    {
      if (current_width + boxes[i].width() > MAX_WIDTH)
        row.flush();
      current_width = row.add_box(boxes[i]);
      ++i;
    }
    if (!row.empty())
      row.flush();

    std::cin >> i;
    iter = child_positions[i];
    board = &iter->first;
  }
#endif

#if 0
  // Generate all positions that are mate in 1, 2, ..., `ply` moves.
  int max_ply = 30;
  //Debug(libcwd::libcw_do.off());
  graph.generate(max_ply);
  //Debug(libcwd::libcw_do.on());

  // All squares that a rook can stand on, given the squares of the kings and the number of moves.
  using rook_positions_type = std::vector<Square>;
  // A map with rook positions as function of kings positions, given the number of moves.
  using kings_to_rooks_map_type = std::map<TwoKings, rook_positions_type>;
  // A vector with a kings_to_rooks_type maps for each given number of moves.
  std::vector<kings_to_rooks_map_type> ply_to_map(max_ply + 1);

  // For a given number of moves that a position must be mate...
  Dout(dc::notice, "The size of ply_to_map is " << ply_to_map.size());
  for (int ply = 4; ply < ply_to_map.size(); ++ply)
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

      Dout(dc::notice, "Black king: " << two_kings.bk() << ", white king: " << two_kings.wk() << ", ply: " << ply);
      Board::utf8art(std::cout, board_size, [&](Square pos) -> Board::Figure {
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
#endif
}
