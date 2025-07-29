#include "sys.h"
#include "Graph.h"
//#include "TwoKings.h"
#include "Box.h"
#include "parse_move.h"
#include <vector>
#include <iostream>
#include <cassert>
#include <variant>
#include <algorithm>
#include "debug.h"

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  constexpr int board_size = version1::Board::board_size;

  // Construct the initial graph with all positions that are already mate.
  version0::Graph graph0(board_size);
  version1::Graph graph1;

  int total_positions0;
  int draw_positions0 = 0;
  int black_in_check_positions0 = 0;
  int mate_positions0 = 0;
  int stalemate_positions0 = 0;

  int total_positions1;
  int draw_positions1 = 0;
  int black_in_check_positions1 = 0;
  int mate_positions1 = 0;
  int stalemate_positions1 = 0;

  // Generate all possible positions.
  graph0.classify();
  graph1.classify();

  std::vector<version0::Graph::black_to_move_nodes_type::iterator> already_mate0;
  std::vector<version1::Graph::black_to_move_nodes_type::iterator> already_mate1;

  // Black to move positions.
  {
    version0::Graph::black_to_move_nodes_type& black_to_move_map = graph0.black_to_move_map();
    total_positions0 = black_to_move_map.size();
    for (version0::Graph::black_to_move_nodes_type::iterator current_position = black_to_move_map.begin();
        current_position != black_to_move_map.end(); ++current_position)
    {
      version0::Board const& current_board = current_position->first;
      version0::BlackToMoveData const& data = current_position->second;
      version0::Classification const& pc = data;
      if (pc.is_draw())
        ++draw_positions0;
      if (pc.is_check())
        ++black_in_check_positions0;
      if (pc.is_mate())
      {
        ++mate_positions0;
        already_mate0.push_back(current_position);
      }
      if (pc.is_stalemate())
        ++stalemate_positions0;
    }
  }
  // White to move positions.
  {
    version0::Graph::white_to_move_nodes_type& white_to_move_map = graph0.white_to_move_map();
    total_positions0 += white_to_move_map.size();
    for (version0::Graph::white_to_move_nodes_type::iterator current_position = white_to_move_map.begin();
        current_position != white_to_move_map.end(); ++current_position)
    {
      version0::Board const& current_board = current_position->first;
      version0::WhiteToMoveData const& data = current_position->second;
      version0::Classification const& pc = data;
      if (pc.is_draw())
        ++draw_positions0;
    }
  }

  std::cout << "Version 0:" << std::endl;
  std::cout << "Total legal positions: " << total_positions0 << std::endl;
  std::cout << "Draw positions: " << draw_positions0 << std::endl;
  std::cout << "Black in check positions: " << black_in_check_positions0 << std::endl;
  std::cout << "Mate positions: " << mate_positions0 << std::endl;
  std::cout << "Stalemate positions: " << stalemate_positions0 << std::endl;

  // Black to move positions.
  {
    version1::Graph::black_to_move_nodes_type& black_to_move_map = graph1.black_to_move_map();
    total_positions1 = black_to_move_map.size();
    for (version1::Graph::black_to_move_nodes_type::iterator current_position = black_to_move_map.begin();
        current_position != black_to_move_map.end(); ++current_position)
    {
      version1::Board const& current_board = current_position->first;
      version1::BlackToMoveData const& data = current_position->second;
      version1::Classification const& pc = data;
      if (pc.is_draw())
        ++draw_positions1;
      if (pc.is_check())
        ++black_in_check_positions1;
      if (pc.is_mate())
      {
        ++mate_positions1;
        already_mate1.push_back(current_position);
      }
      if (pc.is_stalemate())
        ++stalemate_positions1;
    }
  }
  // White to move positions.
  {
    version1::Graph::white_to_move_nodes_type& white_to_move_map = graph1.white_to_move_map();
    total_positions1 += white_to_move_map.size();
    for (version1::Graph::white_to_move_nodes_type::iterator current_position = white_to_move_map.begin();
        current_position != white_to_move_map.end(); ++current_position)
    {
      version1::Board const& current_board = current_position->first;
      version1::WhiteToMoveData const& data = current_position->second;
      version1::Classification const& pc = data;
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
  graph0.generate_edges();
  graph1.generate_edges();

  // Run over all positions that are already mate (as per the classification)
  // and mark all position that can reach those as mate in 1 ply.
  std::vector<version0::Graph::white_to_move_nodes_type::iterator> white_to_move_parents0;
  for (version0::Graph::black_to_move_nodes_type::iterator iter : already_mate0)
  {
    version0::BlackToMoveData& black_to_move_data = iter->second;
    black_to_move_data.set_mate_in_ply(0);
    black_to_move_data.set_maximum_ply_on_parents(white_to_move_parents0);
  }

  // Run over all positions that are already mate (as per the classification)
  // and mark all position that can reach those as mate in 1 ply.
  std::vector<version1::Graph::white_to_move_nodes_type::iterator> white_to_move_parents1;
  for (version1::Graph::black_to_move_nodes_type::iterator iter : already_mate1)
  {
    version1::BlackToMoveData& black_to_move_data = iter->second;
    black_to_move_data.set_mate_in_ply(0);
    black_to_move_data.set_maximum_ply_on_parents(white_to_move_parents1);
  }

  {
    // white_to_move_parents are mate in `ply` moves.
    int ply = 1;
    while (!white_to_move_parents0.empty())
    {
      std::vector<version0::Graph::black_to_move_nodes_type::iterator> black_to_move_parents;
      // Run over all positions that are mate in an odd number of ply.
      for (version0::Graph::white_to_move_nodes_type::iterator white_to_move_board_data_iter : white_to_move_parents0)
      {
        version0::WhiteToMoveData& white_to_move_data = white_to_move_board_data_iter->second;
        ASSERT(white_to_move_data.ply() == ply);
        white_to_move_data.set_minimum_ply_on_parents(black_to_move_parents);
      }
      ++ply;

      if (black_to_move_parents.empty())
        break;

      white_to_move_parents0.clear();
      // Run over all positions that are mate in an even number of ply.
      for (version0::Graph::black_to_move_nodes_type::iterator board_data_iter : black_to_move_parents)
      {
        version0::BlackToMoveData& data = board_data_iter->second;
        ASSERT(data.ply() == ply);
        data.set_maximum_ply_on_parents(white_to_move_parents0);
      }
      ++ply;
    }
    Dout(dc::notice, "ply = " << ply);
  }

  {
    // white_to_move_parents are mate in `ply` moves.
    int ply = 1;
    while (!white_to_move_parents1.empty())
    {
      std::vector<version1::Graph::black_to_move_nodes_type::iterator> black_to_move_parents;
      // Run over all positions that are mate in an odd number of ply.
      for (version1::Graph::white_to_move_nodes_type::iterator white_to_move_board_data_iter : white_to_move_parents1)
      {
        version1::WhiteToMoveData& white_to_move_data = white_to_move_board_data_iter->second;
        ASSERT(white_to_move_data.ply() == ply);
        white_to_move_data.set_minimum_ply_on_parents(black_to_move_parents);
      }
      ++ply;

      if (black_to_move_parents.empty())
        break;

      white_to_move_parents1.clear();
      // Run over all positions that are mate in an even number of ply.
      for (version1::Graph::black_to_move_nodes_type::iterator board_data_iter : black_to_move_parents)
      {
        version1::BlackToMoveData& data = board_data_iter->second;
        ASSERT(data.ply() == ply);
        data.set_maximum_ply_on_parents(white_to_move_parents1);
      }
      ++ply;
    }
    Dout(dc::notice, "ply = " << ply);
  }

  ASSERT(graph0 == graph1);

  // Get a pointer to the initial position.
#if 1
  version1::Graph::white_to_move_nodes_type::const_iterator initial_position = graph1.white_to_move_map().begin();
  while (initial_position->first.black_king() == initial_position->first.white_rook())
    ++initial_position;
#else
  version1::Board b({1, 4}, {4, 2}, {4, 0});
  auto initial_position = graph1.white_to_move_map().find(b);
#endif
  version1::Board const* board = &initial_position->first;

  // Define variants for a single node iterator, and a vector of node iterators.
  using NodeIterator0 = std::variant<
    version1::Graph::black_to_move_nodes_type::const_iterator,
    version1::Graph::white_to_move_nodes_type::const_iterator
  >;

  // Define lambda's for accessing this variants.
  auto get_ply = [](auto&& it) { return it->second.ply(); };
  auto get_classification = [](auto&& it) -> version1::Classification const& { return it->second; };

  using ChildPositions0 = std::variant<
    std::vector<version1::Graph::black_to_move_nodes_type::const_iterator>,
    std::vector<version1::Graph::white_to_move_nodes_type::const_iterator>
  >;

  // Define lambda's for accessing this variants.
  auto get_child_positions = [](auto&& it) -> ChildPositions0 { return it->second.child_positions(); };
  auto get_size = [](auto&& child_positions) { return child_positions.size(); };
  auto get_empty = [](auto&& child_positions) { return child_positions.empty(); };

  // iter points to the current position.
  NodeIterator0 iter = initial_position;
  Color to_move(white);

  for (;;)
  {
    int ply = std::visit(get_ply, iter);

    std::cout << "\nCurrent position (" << to_move << " to move; mate in " << ply << " ply):\n";
    board->utf8art(std::cout);

    if (ply == 0)
    {
      if (std::visit(get_classification, iter).is_mate())
        std::cout << "White won." << std::endl;
      else
        std::cout << "Stalemate." << std::endl;
      break;
    }

    auto child_positions = std::visit(get_child_positions, iter);

    if (to_move == black)
    {
      auto const& cps = std::get<std::vector<version1::Graph::white_to_move_nodes_type::const_iterator>>(child_positions);
      unsigned int max_ply = 0;
      for (auto&& child : cps)
      {
        unsigned ply = child->second.ply();
        if (ply >= max_ply)
        {
          max_ply = ply;
          iter = child;
        }
      }
    }
    else

#if 0
    if (ply == -1 || std::visit(get_empty, child_positions))
    {
      version1::Classification const& classification = std::visit(get_classification, iter);
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
      auto get_classification_i = [i](auto&& child_positions) -> version1::Classification const& { return child_positions[i]->second; };
      version1::Classification const& classification = std::visit(get_classification_i, child_positions);
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
#else
    for (;;)
    {
      auto [piece, x, y] = parse_move(to_move, board_size);
      version1::Board new_board(*board);
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
          auto found = std::find_if(vec.begin(), vec.end(), [&](auto&& board){ return board->first == new_board; });
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
#endif
    board = std::visit([](auto&& it) { return &it->first; }, iter);
    to_move = to_move.opponent();
  }
}
