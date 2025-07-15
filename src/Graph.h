#pragma once

#include "Board.h"
#include "Data.h"
#include <map>
#include "debug.h"

struct DistanceCompare
{
  bool operator()(Board const& lhs, Board const& rhs) const
  {
    return lhs.distance_less(rhs);
  }
};

class Graph
{
 public:
  using nodes_type = std::map<Board, Data, DistanceCompare>;
  using positions_type = std::vector<nodes_type::const_iterator>;       // Stores iterators to positions that are mate in a given number of ply.
  using mate_in_ply_type = std::vector<positions_type>;                 // Store all boards that are mate in `ply` at index `ply`.

 private:
  nodes_type nodes_;
  mate_in_ply_type mate_in_ply_;

 public:
  Graph();

  void generate(int ply);

  positions_type const& mate_in_ply(int ply) const
  {
    // Call generate(ply) first.
    ASSERT(ply < mate_in_ply_.size());
    return mate_in_ply_[ply];
  }

 public:
  // Generate all positions that can reach position of board in one ply.
  std::vector<Board> preceding_positions(Board const& board);
};
