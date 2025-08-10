#include "sys.h"
#include "Graph.h"
#include "parse_move.h"
#include <bitset>
#include "debug.h"

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  Dout(dc::notice, "sizeof(Info) = " << sizeof(Info));

  // Get the size of the board.
  int const board_size_x = Size::board::x;
  int const board_size_y = Size::board::y;
  Dout(dc::notice, "Board size: " << board_size_x << "x" << board_size_y);

  // Get the size of a block.
  int const block_size_x = Size::block::x;
  int const block_size_y = Size::block::y;
  Dout(dc::notice, "Block size: " << block_size_x << "x" << block_size_y);

  // Construct the initial graph with all positions that are already mate.
  auto start2 = std::chrono::high_resolution_clock::now();

  Graph graph2;

  int total_positions2;
  int draw_positions2 = 0;
  int black_in_check_positions2 = 0;
  int mate_positions2 = 0;
  int stalemate_positions2 = 0;

  // Generate all possible positions.
  graph2.classify();

  std::vector<InfoIndex> already_mate2;

  // Black to move positions.
  {
    Graph::info_nodes_type& black_to_move = graph2.black_to_move();
    total_positions2 = 0;
    for (Graph::info_nodes_type::index_type current_info_index = black_to_move.ibegin();
        current_info_index != black_to_move.iend(); ++current_info_index)
    {
      Info const& info = black_to_move[current_info_index];
      Classification const& pc = info.classification();
      if (!pc.is_legal())
        continue;
      ++total_positions2;
      if (pc.is_draw())
        ++draw_positions2;
      if (pc.is_check())
        ++black_in_check_positions2;
      if (pc.is_mate())
      {
        ++mate_positions2;
        already_mate2.push_back(current_info_index);
      }
      if (pc.is_stalemate())
        ++stalemate_positions2;
    }
  }
  // White to move positions.
  {
    Graph::info_nodes_type& white_to_move = graph2.white_to_move();
    // Run over all legal positions in white_to_move_map.
    for (Graph::info_nodes_type::index_type current_info_index = white_to_move.ibegin();
        current_info_index != white_to_move.iend(); ++current_info_index)
    {
      Info const& info = white_to_move[current_info_index];
      Classification const& pc = info.classification();
      if (!pc.is_legal())
        continue;
      ++total_positions2;
      if (pc.is_draw())
        ++draw_positions2;
    }
  }

  std::cout << "Version 2:" << std::endl;
  std::cout << "Total legal positions: " << total_positions2 << std::endl;
  std::cout << "Draw positions: " << draw_positions2 << std::endl;
  std::cout << "Black in check positions: " << black_in_check_positions2 << std::endl;
  std::cout << "Mate positions: " << mate_positions2 << std::endl;
  std::cout << "Stalemate positions: " << stalemate_positions2 << std::endl;

  auto end2 = std::chrono::high_resolution_clock::now();
  auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
  std::cout << "Execution time: " << (duration2.count() / 1000000.0) << " seconds\n";

  // Run over all positions that are already mate (as per the classification)
  // and mark all position that can reach those as mate in 1 ply.
  std::vector<Board> white_to_move_parents2;
  Graph::info_nodes_type& black_to_move = graph2.black_to_move();
  for (Graph::info_nodes_type::index_type info_index : already_mate2)
  {
    Info& black_to_move_info = black_to_move[info_index];
    black_to_move_info.set_mate_in_ply(0);

    Board b(info_index);
    b.utf8art(std::cout, black);
//FIXME    black_to_move_info.set_maximum_ply_on_parents(white_to_move_parents2);
  }

#if 0
  // Board::inc_field<0, bkbi>() with this = [{black king:(10, 3), w
  Board board({7, 9}, {6, 7}, {6, 14});
  Color to_move = black;
  std::cout << "Original position:\n";
  board.utf8art(std::cout, to_move, false);
  Board::neighbors_type neighbors;
  int n = board.generate_neighbors(to_move, Board::children, neighbors);
  std::cout << "Child positions:\n";
  for (int i = 0; i < n; ++i)
    neighbors[i].utf8art(std::cout, to_move.opponent(), false, board.black_king());
#endif

#if 0
  Board board({4, 5}, {7, 3}, {0, 1});
  Color to_move(white);
  int ply = -1;
  for (;;)
  {
    std::cout << "\nCurrent position (" << to_move << " to move; mate in " << ply << " ply):\n";
    board.utf8art(std::cout, to_move);

    // Get a move from the user.
    for (;;)
    {
      auto [piece, x, y] = parse_move(to_move, board_size_x, board_size_y);
      Board new_board(board);
      if (to_move == white)
      {
        if (piece == 'R')
          new_board.set_white_rook_square({x, y});
        else
          new_board.set_white_king_square({x, y});
      }
      else if (piece == 'K')
        new_board.set_black_king_square({x, y});

      board = new_board;
      break;
    }

    to_move = to_move.opponent();
  }
#endif
}
