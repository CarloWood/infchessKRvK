#pragma once

#include "Board.h"
#include <vector>
#include <map>

class Data
{
 public:
  // Must be the same type as Graph::nodes_type.
  using nodes_type = std::map<Board, Data, DistanceCompare>;

 private:
  int ply_;     // Number of moves (in ply) required to reach mate under optimal play.
  std::vector<nodes_type::const_iterator> possible_moves_;

 public:
  Data(int ply) : ply_(ply) { }

  // Accessors.
  int ply() const { return ply_; }
  std::vector<nodes_type::const_iterator> const& possible_moves() const { return possible_moves_; }

  void add_edge(nodes_type::const_iterator const& next_board);
};
