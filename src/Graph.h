#pragma once

#include "Data.h"
#include "Color.h"
#include "debug.h"

class Graph
{
 public:
  using nodes_type = Data::nodes_type;
  using positions_type = std::vector<nodes_type::const_iterator>;       // Stores iterators to positions that are mate in a given number of ply.
  using mate_in_ply_type = std::vector<positions_type>;                 // Store all boards that are mate in `ply` at index `ply`.

 private:
  int board_size_;
  nodes_type black_to_move_;
  nodes_type white_to_move_;
  mate_in_ply_type mate_in_ply_;

 public:
  Graph(int board_size);

  // Accessors (for debugging purposes).
  nodes_type& black_to_move_map() { return black_to_move_; }
  nodes_type& white_to_move_map() { return white_to_move_; }
  nodes_type const& black_to_move_map() const { return black_to_move_; }
  nodes_type const& white_to_move_map() const { return white_to_move_; }

  void classify();
//  void generate(int ply);
  void generate_edges();

  positions_type const& mate_in_ply(int ply) const
  {
    // Call generate(ply) first.
    ASSERT(ply < mate_in_ply_.size());
    return mate_in_ply_[ply];
  }

 public:
  // Generate all positions that `to_move` can reach the position of `board` in one ply.
//  std::vector<Board> adjacent_positions(Board const& board, Color to_move);
};
