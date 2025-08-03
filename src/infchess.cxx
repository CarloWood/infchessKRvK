#include "sys.h"
#include "Graph.h"
#include "Box.h"
#include "parse_move.h"
#include "print_formula_table.h"
#include <vector>
#include <iostream>
#include <cassert>
#include <variant>
#include <algorithm>
#include <chrono>
#include "debug.h"

Board get_relative_position(int relative_position_index, int n, int m)
{
  //       0 1 2 <- white king
  //   m ─   R 3
  // m-1 ─ k   4
  //       | |
  //       n n+1

  int wkx = n + std::min(relative_position_index, 2);
  int wky = m + 3 - std::max(relative_position_index, 2);
  Board result({n, m - 1}, {wkx, wky}, {n + 1, m});

  return result;
}

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  constexpr int board_size_x = Board::board_size_x;
  constexpr int board_size_y = Board::board_size_y;

  // Construct the initial graph with all positions that are already mate.
  auto start1 = std::chrono::high_resolution_clock::now();

  Graph graph1;

  int total_positions1;
  int draw_positions1 = 0;
  int black_in_check_positions1 = 0;
  int mate_positions1 = 0;
  int stalemate_positions1 = 0;

  // Generate all possible positions.
  graph1.classify();

  std::vector<Graph::black_to_move_nodes_type::iterator> already_mate1;

  // Black to move positions.
  {
    Graph::black_to_move_nodes_type& black_to_move_map = graph1.black_to_move_map();
    total_positions1 = 0;
    for (Graph::black_to_move_nodes_type::iterator current_data = black_to_move_map.begin();
        current_data != black_to_move_map.end(); ++current_data)
    {
      BlackToMoveData const& data = *current_data;
      Classification const& pc = data;
      if (!pc.is_legal())
        continue;
      ++total_positions1;
      if (pc.is_draw())
        ++draw_positions1;
      if (pc.is_check())
        ++black_in_check_positions1;
      if (pc.is_mate())
      {
        ++mate_positions1;
        already_mate1.push_back(current_data);
      }
      if (pc.is_stalemate())
        ++stalemate_positions1;
    }
  }
  // White to move positions.
  {
    Graph::white_to_move_nodes_type& white_to_move_map = graph1.white_to_move_map();
    // Run over all legal positions in white_to_move_map.
    for (Graph::white_to_move_nodes_type::iterator current_data = white_to_move_map.begin();
        current_data != white_to_move_map.end(); ++current_data)
    {
      WhiteToMoveData const& data = *current_data;
      Classification const& pc = data;
      if (!pc.is_legal())
        continue;
      ++total_positions1;
      if (pc.is_draw())
        ++draw_positions1;
    }
  }

  std::cout << "Version 1:" << std::endl;
  std::cout << "Total legal positions: " << total_positions1 << std::endl;
  std::cout << "Draw positions: " << draw_positions1 << std::endl;
  std::cout << "Black in check positions: " << black_in_check_positions1 << std::endl;
  std::cout << "Mate positions: " << mate_positions1 << std::endl;
  std::cout << "Stalemate positions: " << stalemate_positions1 << std::endl;

  // Generate links for all legal moves.
  graph1.generate_edges();

  // Run over all positions that are already mate (as per the classification)
  // and mark all position that can reach those as mate in 1 ply.
  std::vector<Graph::white_to_move_nodes_type::iterator> white_to_move_parents1;
  for (Graph::black_to_move_nodes_type::iterator iter : already_mate1)
  {
    BlackToMoveData& black_to_move_data = *iter;
    black_to_move_data.set_mate_in_ply(0);
    black_to_move_data.set_maximum_ply_on_parents(white_to_move_parents1);
  }

  Graph::white_to_move_nodes_type::const_iterator initial_position;
  {
    // white_to_move_parents are mate in `ply` moves.
    int ply = 1;
    while (!white_to_move_parents1.empty())
    {
      std::vector<Graph::black_to_move_nodes_type::iterator> black_to_move_parents;
      // Run over all positions that are mate in an odd number of ply.
      for (Graph::white_to_move_nodes_type::iterator white_to_move_board_data_iter : white_to_move_parents1)
      {
        WhiteToMoveData& white_to_move_data = *white_to_move_board_data_iter;
        ASSERT(white_to_move_data.ply() == ply);
        white_to_move_data.set_minimum_ply_on_parents(black_to_move_parents);
      }
      ++ply;

      if (black_to_move_parents.empty())
        break;

      white_to_move_parents1.clear();
      // Run over all positions that are mate in an even number of ply.
      for (Graph::black_to_move_nodes_type::iterator data_iter : black_to_move_parents)
      {
        BlackToMoveData& data = *data_iter;
        ASSERT(data.ply() == ply);
        data.set_maximum_ply_on_parents(white_to_move_parents1);
      }
      ++ply;

      if (!white_to_move_parents1.empty())
      {
        initial_position = white_to_move_parents1[0];
      }
    }
    Dout(dc::notice, "ply = " << ply);
  }

  auto end1 = std::chrono::high_resolution_clock::now();
  auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1);
  std::cout << "Execution time: " << (duration1.count() / 1000000.0) << " seconds\n";

  Graph const& graph = graph1;

  // Get a pointer to the initial position.
#if -0
  // Use initial_position initialized above with the position that is mate in the maximum number of ply.
#elif 0
  // Use the first legal position with the white rook on the board.
  initial_position = graph.white_to_move_map().begin();
  while (
      !initial_position->is_legal() ||
      Board{graph.get_index(initial_position)}.black_king() == Board{graph.get_index(initial_position)}.white_rook()
  )
    ++initial_position;
#elif 1
  for (int m = 2; m < 7; ++m)
  {
    std::vector<int> black_to_move_final_k_values;
    std::vector<int> white_to_move_final_k_values;

    for (int relative_position_index = 0; relative_position_index < 5; ++relative_position_index)
    {
      int const offset = 2;     // Using a value of 2 keeps the white rook away from the virtual right edge.
      Board const largest_n_board = get_relative_position(relative_position_index, board_size_x - 1 - offset, m);
      {
        constexpr Color to_move = black;
        largest_n_board.utf8art(std::cout, to_move, true);
        std::cout << "\n";
        std::vector<int> ply_values;
        for (int n = 0; n < board_size_x - offset; ++n)
        {
          Board b = get_relative_position(relative_position_index, n, m);
          ply_values.push_back(graph.map_with_to_move<to_move.color_>()[b.as_index()].ply());
        }
        int final_k = print_formula_table(m, ply_values);
        black_to_move_final_k_values.push_back(final_k);
        std::cout << "\n";
      }
      {
        constexpr Color to_move = white;
        largest_n_board.utf8art(std::cout, to_move, true);
        std::cout << "\n";
        std::vector<int> ply_values;
        for (int n = 0; n < board_size_x - offset; ++n)
        {
          Board b = get_relative_position(relative_position_index, n, m);
          ply_values.push_back(graph.map_with_to_move<to_move.color_>()[b.as_index()].ply());
        }
        int final_k = print_formula_table(m, ply_values);
        white_to_move_final_k_values.push_back(final_k);
        std::cout << "\n";
      }
    }

    std::cout << "m=" << m <<
                    "   ┏━━━━━┯━━━━━┯━━━━━┯━━━━━┯━━━━━┓\n";
    std::cout << "      ┃K   ·│· K ·│·   K│·   ·│·   ·┃\n";
    std::cout << "  to  ┃  R  │  R  │  R  │  R K│  R  ┃\n";
    std::cout << " move ┃k   ·│k   ·│k   ·│k   ·│k   K┃\n";
    std::cout << "┏━━━━━╋━━━━━┿━━━━━┿━━━━━┿━━━━━┿━━━━━┫\n";
    std::cout << "┃black┃";
    char const* separator = "";
    // Print "12+2n│12+2n│14+2n│12+2n│ 8+2n"
    for (int k : black_to_move_final_k_values)
    {
      std::cout << separator << std::setw(2) << k << "+2n";
      separator = "│";
    }
    std::cout <<                                     "┃\n";
    std::cout << "┠─────╂─────┼─────┼─────┼─────┼─────┨\n";
    std::cout << "┃white┃";
    separator = "";
    // Print "11+2n│13+2n│13+2n│ 9+2n│11+2n"
    for (int k : white_to_move_final_k_values)
    {
      std::cout << separator << std::setw(2) << k << "+2n";
      separator = "│";
    }
    std::cout <<                                     "┃\n";
    std::cout << "┗━━━━━┻━━━━━┷━━━━━┷━━━━━┷━━━━━┷━━━━━┛\n";
  }

#endif
  //return 0;

  int n = 7;
  int m = 6;
  Board b({n, m - 1}, {n, m + 1}, {n + 1, m});
  //Board b({3, 1}, {3, 3}, {4, 2});
  initial_position = graph.white_to_move_map().begin() + b.as_index();
  Board board1(graph.get_index(initial_position));
  Board const* board = &board1;

#if 1

  // Define variants for a single node iterator, and a vector of node iterators.
  using NodeIterator0 = std::variant<
    Graph::black_to_move_nodes_type::const_iterator,
    Graph::white_to_move_nodes_type::const_iterator
  >;

  // Define lambda's for accessing this variants.
  auto get_ply = [](auto&& it) { return it->ply(); };
  auto get_classification = [](auto&& it) -> Classification const& { return *it; };

  using ChildPositions0 = std::variant<
    std::vector<Graph::black_to_move_nodes_type::const_iterator>,
    std::vector<Graph::white_to_move_nodes_type::const_iterator>
  >;

  // Define lambda's for accessing this variants.
  auto get_child_positions = [](auto&& it) -> ChildPositions0 { return it->child_positions(); };
  auto get_size = [](auto&& child_positions) { return child_positions.size(); };
  auto get_empty = [](auto&& child_positions) { return child_positions.empty(); };

  // iter points to the current position.
  NodeIterator0 iter = initial_position;

  for (;;)
  {
    int ply = std::visit(get_ply, iter);
    Color to_move(ply % 2 == 0 ? black : white);

    std::cout << "\nCurrent position (" << to_move << " to move; mate in " << ply << " ply):\n";
    board->utf8art(std::cout, to_move);

    if (ply <= 0)
    {
      if (std::visit(get_classification, iter).is_mate())
        std::cout << "White won." << std::endl;
      else if (std::visit(get_classification, iter).is_stalemate())
        std::cout << "Stalemate." << std::endl;
      else if (std::visit(get_classification, iter).is_draw())
        std::cout << "Draw." << std::endl;
      break;
    }

    auto child_positions = std::visit(get_child_positions, iter);

    // Print all moves that are "best play".
    {
      unsigned int min_max_ply = to_move == black ? 0 : -1;
      for (int i = 0; i < std::visit(get_size, child_positions); ++i)
      {
        auto get_classification_i = [i](auto&& child_positions) -> Classification const& { return *child_positions[i]; };
        Classification const& classification = std::visit(get_classification_i, child_positions);
        unsigned int ply = classification.ply();
        if (to_move == black)
        {
          // Black to move, go for the maximum ply.
          if (ply > min_max_ply)
            min_max_ply = ply;
        }
        else
        {
          // White to move, go for the minimum ply.
          if (ply < min_max_ply)
            min_max_ply = ply;
        }
      }
      char const* separator = "";
      for (int i = 0; i < std::visit(get_size, child_positions); ++i)
      {
        auto get_classification_i = [i](auto&& child_positions) -> Classification const& { return *child_positions[i]; };
        Classification const& classification = std::visit(get_classification_i, child_positions);
        int ply = classification.ply();
        if (ply == min_max_ply)
        {
          auto get_board = [&graph, i](auto&& child_positions) -> Board { return {graph.get_index(child_positions[i])}; };
          std::cout << separator << board1.get_move(std::visit(get_board, child_positions));
          separator = ", ";
        }
      }
      std::cout << std::endl;
    }

    if (0) //(to_move == black)
    {
      auto const& cps = std::get<std::vector<Graph::white_to_move_nodes_type::const_iterator>>(child_positions);
      unsigned int max_ply = 0;
      for (auto&& child : cps)
      {
        unsigned ply = child->ply();
        if (ply >= max_ply)
        {
          max_ply = ply;
          iter = child;
        }
      }
    }
    else
#if -0
    {
      // Print all child positions and let the user pick one.
      if (ply == -1 || std::visit(get_empty, child_positions))
      {
        Classification const& classification = std::visit(get_classification, iter);
        if (classification.is_mate())
        {
          std::cout << "You won!" << std::endl;
          break;
        }
        else if (classification.is_stalemate())
        {
          std::cout << "Oops, stalemate!" << std::endl;
          break;
        }
        else if (classification.is_draw())
        {
          std::cout << "Sorry, draw!" << std::endl;
          break;
        }
        else if (ply != -1)
          break;
        std::cout << "Black will keep this a draw!" << std::endl;
      }

      std::vector<Box> boxes;
      for (int i = 0; i < std::visit(get_size, child_positions); ++i)
      {
        boxes.emplace_back();
        boxes.back().stream() << "\n" << i << ":\n";
        auto get_board = [i](auto&& child_positions) { return child_positions[i]->first; };
        std::visit(get_board, child_positions).utf8art(boxes.back().stream());
        auto get_classification_i = [i](auto&& child_positions) -> Classification const& { return child_positions[i]->second; };
        Classification const& classification = std::visit(get_classification_i, child_positions);
        if (classification.is_stalemate())
          boxes.back().stream() << "Stalemate";
        else if (classification.ply() == -1)
          boxes.back().stream() << "Draw";
        else if (classification.ply() == 0)
          boxes.back().stream() << "Mate!";
        else
          boxes.back().stream() << "Mate in " << classification.ply() << " ply";
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
      iter = std::visit([i](auto&& vec) -> NodeIterator0 { return vec[i]; }, child_positions);
    }
#else
    {
      // Get a move from the user.
      for (;;)
      {
        auto [piece, x, y] = parse_move(to_move, board_size_x, board_size_y);
        Board new_board(*board);
        if (to_move == white)
        {
          if (piece == 'R')
            new_board.set_white_rook_square({x, y});
          else
            new_board.set_white_king_square({x, y});
        }
        else if (piece == 'K')
          new_board.set_black_king_square({x, y});

        try
        {
          iter = std::visit([&](auto&& vec) -> NodeIterator0 {
            auto new_board_index = new_board.as_index();
            auto found = std::find_if(vec.begin(), vec.end(), [&](auto&& board){ return graph.get_index(board) == new_board_index; });
            if (found == vec.end())
              throw std::invalid_argument("Illegal move");
            return *found;
          }, child_positions);
        }
        catch (std::exception const& e)
        {
          std::cerr << "Error: " << e.what() << std::endl;
          continue;
        }
        break;
      }
    }
#endif
    board1 = std::visit([&](auto&& it) -> Board { return graph.get_index(it); }, iter);
    to_move = to_move.opponent();
  }
#endif
}
