#include "sys.h"
#include "Graph.h"
//#include "TwoKings.h"
//#include "Box.h"
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

  int total_positions;
  int draw_positions = 0;
  int black_in_check_positions = 0;
  int mate_positions = 0;
  int stalemate_positions = 0;

  // Generate all possible positions.
  graph0.classify();
  graph1.classify();

  std::vector<version0::Graph::black_to_move_nodes_type::iterator> already_mate;

  // Black to move positions.
  {
    version0::Graph::black_to_move_nodes_type& black_to_move_map = graph0.black_to_move_map();
    total_positions = black_to_move_map.size();
    for (version0::Graph::black_to_move_nodes_type::iterator current_position = black_to_move_map.begin();
        current_position != black_to_move_map.end(); ++current_position)
    {
      version0::Board const& current_board = current_position->first;
      version0::BlackToMoveData const& data = current_position->second;
      version0::Classification const& pc = data;
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
    version0::Graph::white_to_move_nodes_type& white_to_move_map = graph0.white_to_move_map();
    total_positions += white_to_move_map.size();
    for (version0::Graph::white_to_move_nodes_type::iterator current_position = white_to_move_map.begin();
        current_position != white_to_move_map.end(); ++current_position)
    {
      version0::Board const& current_board = current_position->first;
      version0::WhiteToMoveData const& data = current_position->second;
      version0::Classification const& pc = data;
      if (pc.is_draw())
        ++draw_positions;
    }
  }

  std::cout << "Total legal positions: " << total_positions << std::endl;
  std::cout << "Draw positions: " << draw_positions << std::endl;
  std::cout << "Black in check positions: " << black_in_check_positions << std::endl;
  std::cout << "Mate positions: " << mate_positions << std::endl;
  std::cout << "Stalemate positions: " << stalemate_positions << std::endl;

  ASSERT(graph0 == graph1);

  // Generate links for all legal moves.
  graph0.generate_edges();

  // Run over all positions that are already mate (as per the classification)
  // and mark all position that can reach those as mate in 1 ply.
  std::vector<version0::Graph::white_to_move_nodes_type::iterator> white_to_move_parents;
  for (version0::Graph::black_to_move_nodes_type::iterator iter : already_mate)
  {
    version0::BlackToMoveData& black_to_move_data = iter->second;
    black_to_move_data.set_mate_in_ply(0);
    black_to_move_data.set_maximum_ply_on_parents(white_to_move_parents);
  }

  {
    // white_to_move_parents are mate in `ply` moves.
    int ply = 1;
    while (!white_to_move_parents.empty())
    {
      std::vector<version0::Graph::black_to_move_nodes_type::iterator> black_to_move_parents;
      // Run over all positions that are mate in an odd number of ply.
      for (version0::Graph::white_to_move_nodes_type::iterator white_to_move_board_data_iter : white_to_move_parents)
      {
        version0::WhiteToMoveData& white_to_move_data = white_to_move_board_data_iter->second;
        ASSERT(white_to_move_data.ply() == ply);
        white_to_move_data.set_minimum_ply_on_parents(black_to_move_parents);
      }
      ++ply;

      if (black_to_move_parents.empty())
        break;

      white_to_move_parents.clear();
      // Run over all positions that are mate in an even number of ply.
      for (version0::Graph::black_to_move_nodes_type::iterator board_data_iter : black_to_move_parents)
      {
        version0::BlackToMoveData& data = board_data_iter->second;
        ASSERT(data.ply() == ply);
        data.set_maximum_ply_on_parents(white_to_move_parents);
      }
      ++ply;
    }
    Dout(dc::notice, "ply = " << ply);
  }

  // Get a pointer to the initial position.
#if 0
  version0::Graph::white_to_move_nodes_type::const_iterator initial_position = graph0.white_to_move_map().begin();
  while (initial_position->first.black_king() == initial_position->first.white_rook())
    ++initial_position;
#else
  version0::Board b(board_size, {1, 5}, {4, 2}, {4, 0});
  auto initial_position = graph0.white_to_move_map().find(b);
#endif
  version0::Board const* board = &initial_position->first;

  // Define variants for a single node iterator, and a vector of node iterators.
  using NodeIterator0 = std::variant<
    version0::Graph::black_to_move_nodes_type::const_iterator,
    version0::Graph::white_to_move_nodes_type::const_iterator
  >;

  // Define lambda's for accessing this variants.
  auto get_ply = [](auto&& it) { return it->second.ply(); };
  auto get_classification = [](auto&& it) -> version0::Classification const& { return it->second; };

  using ChildPositions0 = std::variant<
    std::vector<version0::Graph::black_to_move_nodes_type::const_iterator>,
    std::vector<version0::Graph::white_to_move_nodes_type::const_iterator>
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
      auto const& cps = std::get<std::vector<version0::Graph::white_to_move_nodes_type::const_iterator>>(child_positions);
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
#else
    for (;;)
    {
      auto [piece, x, y] = parse_move(to_move, board_size);
      version0::Board new_board(*board);
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

#if 0
  // Generate all positions that are mate in 1, 2, ..., `ply` moves.
  int max_ply = 30;
  //Debug(libcwd::libcw_do.off());
  graph0.generate(max_ply);
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
    for (version0::Graph::nodes_type::const_iterator const& mate_in_ply_position : graph0.mate_in_ply(ply))
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
      version0::Board::utf8art(std::cout, board_size, [&](Square pos) -> version0::Board::Figure {
        if (pos == two_kings.bk())
          return version0::Board::Figure::black_king;
        else if (pos == two_kings.wk())
          return version0::Board::Figure::white_king;
        else
        {
          for (int i = 0; i < rook_positions.size(); ++i)
            if (pos == rook_positions[i])
              return version0::Board::Figure::white_rook;
        }
        return version0::Board::Figure::none;
      });
      std::cout << "Mate in " << ply << " ply";
      if (ply % 2 == 0)
        std::cout << " (black to move)";
      std::cout << std::endl;
    }
  }
#endif
}
