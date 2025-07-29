#pragma once

#include "BlackToMoveData.h"
#include "WhiteToMoveData.h"
#include "Color.h"
#include "debug.h"

namespace version0 {

class Graph
{
 public:
  using black_to_move_nodes_type = BlackToMoveData::black_to_move_nodes_type;
  using white_to_move_nodes_type = WhiteToMoveData::white_to_move_nodes_type;
  // Stores iterators to positions that are mate in a given (even) number of ply.
  using black_to_move_positions_type = std::vector<black_to_move_nodes_type::const_iterator>;
  // Stores iterators to positions that are mate in a given (odd) number of ply.
  using white_to_move_positions_type = std::vector<white_to_move_nodes_type::const_iterator>;
  // Store all boards with black to move that are mate in (the even) `ply` at index `ply / 2`.
  using black_to_move_mate_in_ply_type = std::vector<black_to_move_positions_type>;
  // Store all boards with white to move that are mate in (the odd) `ply` at index `ply / 2`.
  using white_to_move_mate_in_ply_type = std::vector<white_to_move_positions_type>;

 private:
  int board_size_;
  black_to_move_nodes_type black_to_move_;
  white_to_move_nodes_type white_to_move_;
  black_to_move_mate_in_ply_type black_to_move_mate_in_ply_;
  white_to_move_mate_in_ply_type white_to_move_mate_in_ply_;

 public:
  Graph(int board_size);

  // Accessors (for debugging purposes).
  black_to_move_nodes_type& black_to_move_map() { return black_to_move_; }
  white_to_move_nodes_type& white_to_move_map() { return white_to_move_; }
  black_to_move_nodes_type const& black_to_move_map() const { return black_to_move_; }
  white_to_move_nodes_type const& white_to_move_map() const { return white_to_move_; }
  int board_size() const { return board_size_; }

  template<color_type color>
  auto& map_with_to_move()
  {
    if constexpr (color == black)
      return black_to_move_;
    else
      return white_to_move_;
  }

  void classify();
//  void generate(int ply);
  template<color_type color>
  void generate_edges_with_color();
  void generate_edges();

  black_to_move_positions_type const& black_to_move_mate_in_ply(int ply) const
  {
    // With black to move, ply is expected to be even!
    ASSERT(ply % 2 == 0);
    // The index to use.
    int i = ply / 2;
    // Call generate(ply) first.
    ASSERT(i < black_to_move_mate_in_ply_.size());
    return black_to_move_mate_in_ply_[i];
  }

  white_to_move_positions_type const& white_to_move_mate_in_ply(int ply) const
  {
    // With white to move, ply is expected to be odd!
    ASSERT(ply % 2 == 1);
    // The index to use.
    int i = ply / 2;
    // Call generate(ply) first.
    ASSERT(i < white_to_move_mate_in_ply_.size());
    return white_to_move_mate_in_ply_[i];
  }

 public:
  // Generate all positions that `to_move` can reach the position of `board` in one ply.
//  std::vector<Board> adjacent_positions(Board const& board, Color to_move);
};

template<color_type color>
void Graph::generate_edges_with_color()
{
  DoutEntering(dc::notice, "version0::Graph::generate_edges_with_color<" << color << ">()");
  constexpr Color to_move(color);
  // The map containing all positions with `color` to move.
  auto& color_to_move_map = map_with_to_move<color>();
  // The map containing all positions with the other color to move.
  auto& other_to_move_map = map_with_to_move<to_move.opponent().color_>();
  Dout(dc::notice, "Number of positions in color_to_move_map: " << color_to_move_map.size());
  Dout(dc::notice, "Number of positions in other_to_move_map: " << other_to_move_map.size());

  Dout(dc::notice, "Looping over all positions in color_to_move_map:");
  NAMESPACE_DEBUG::Mark mark;
  for (auto current_position = color_to_move_map.begin(); current_position != color_to_move_map.end(); ++current_position)
  {
    Board const& current_board = current_position->first;
    auto& data = current_position->second;
    Dout(dc::notice, "current_board = " << current_board << ", with data = " << data);
    std::vector<Board> succeeding_boards = current_board.get_succeeding_boards(to_move);
    Dout(dc::notice, "current_board.get_succeeding_boards(" << to_move << ") returned " << succeeding_boards.size() << " positions:");
    NAMESPACE_DEBUG::Mark mark2;
    for (Board const& succeeding_board : succeeding_boards)
    {
      Dout(dc::notice, succeeding_board);
      auto child_position = other_to_move_map.find(succeeding_board);
      if (child_position == other_to_move_map.end())
        continue;     // Non-existing boards are illegal.
      data.add_edges(current_position, child_position);
    }
  }
}

} // namespace version0

namespace version1 {

class Graph
{
 public:
  using black_to_move_nodes_type = BlackToMoveData::black_to_move_nodes_type;
  using white_to_move_nodes_type = WhiteToMoveData::white_to_move_nodes_type;
  // Stores iterators to positions that are mate in a given (even) number of ply.
  using black_to_move_positions_type = std::vector<black_to_move_nodes_type::const_iterator>;
  // Stores iterators to positions that are mate in a given (odd) number of ply.
  using white_to_move_positions_type = std::vector<white_to_move_nodes_type::const_iterator>;
  // Store all boards with black to move that are mate in (the even) `ply` at index `ply / 2`.
  using black_to_move_mate_in_ply_type = std::vector<black_to_move_positions_type>;
  // Store all boards with white to move that are mate in (the odd) `ply` at index `ply / 2`.
  using white_to_move_mate_in_ply_type = std::vector<white_to_move_positions_type>;

 private:
  black_to_move_nodes_type black_to_move_;
  white_to_move_nodes_type white_to_move_;
  black_to_move_mate_in_ply_type black_to_move_mate_in_ply_;
  white_to_move_mate_in_ply_type white_to_move_mate_in_ply_;

 public:
  // Accessors (for debugging purposes).
  black_to_move_nodes_type& black_to_move_map() { return black_to_move_; }
  white_to_move_nodes_type& white_to_move_map() { return white_to_move_; }
  black_to_move_nodes_type const& black_to_move_map() const { return black_to_move_; }
  white_to_move_nodes_type const& white_to_move_map() const { return white_to_move_; }

  template<color_type color>
  auto& map_with_to_move()
  {
    if constexpr (color == black)
      return black_to_move_;
    else
      return white_to_move_;
  }

  void classify();
//  void generate(int ply);
  template<color_type color>
  void generate_edges_with_color();
  void generate_edges();

  black_to_move_positions_type const& black_to_move_mate_in_ply(int ply) const
  {
    // With black to move, ply is expected to be even!
    ASSERT(ply % 2 == 0);
    // The index to use.
    int i = ply / 2;
    // Call generate(ply) first.
    ASSERT(i < black_to_move_mate_in_ply_.size());
    return black_to_move_mate_in_ply_[i];
  }

  white_to_move_positions_type const& white_to_move_mate_in_ply(int ply) const
  {
    // With white to move, ply is expected to be odd!
    ASSERT(ply % 2 == 1);
    // The index to use.
    int i = ply / 2;
    // Call generate(ply) first.
    ASSERT(i < white_to_move_mate_in_ply_.size());
    return white_to_move_mate_in_ply_[i];
  }
};

template<color_type color>
void Graph::generate_edges_with_color()
{
  DoutEntering(dc::notice, "version1::Graph::generate_edges_with_color<" << color << ">()");
  constexpr Color to_move(color);
  // The map containing all positions with `color` to move.
  auto& color_to_move_map = map_with_to_move<color>();
  // The map containing all positions with the other color to move.
  auto& other_to_move_map = map_with_to_move<to_move.opponent().color_>();
  Dout(dc::notice, "Number of positions in color_to_move_map: " << color_to_move_map.size());
  Dout(dc::notice, "Number of positions in other_to_move_map: " << other_to_move_map.size());

  Dout(dc::notice, "Looping over all positions in color_to_move_map:");
  NAMESPACE_DEBUG::Mark mark;
  for (auto current_position = color_to_move_map.begin(); current_position != color_to_move_map.end(); ++current_position)
  {
    Board const& current_board = current_position->first;
    auto& data = current_position->second;
    Dout(dc::notice, "current_board = " << current_board << ", with data = " << data);
    std::vector<Board> succeeding_boards = current_board.get_succeeding_boards(to_move);
    Dout(dc::notice, "current_board.get_succeeding_boards(" << to_move << ") returned " << succeeding_boards.size() << " positions:");
    NAMESPACE_DEBUG::Mark mark2;
    for (Board const& succeeding_board : succeeding_boards)
    {
      Dout(dc::notice, succeeding_board);
      auto child_position = other_to_move_map.find(succeeding_board);
      if (child_position == other_to_move_map.end())
        continue;     // Non-existing boards are illegal.
      data.add_edges(current_position, child_position);
    }
  }
}

} // namespace version1

bool operator==(version0::Graph const& lhs, version1::Graph const& rhs);
