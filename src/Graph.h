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
  using map_type = std::map<Board, Data, DistanceCompare>;
  using data_type = std::vector<map_type>;                      // Stores a map as function of the ply in which a position is mate.

 private:
  data_type data_;

 public:
  Graph();

  void generate(int ply);

  map_type const& mate_in_ply(int ply) const
  {
    // Call generate(ply) first.
    ASSERT(ply < data_.size());
    return data_[ply];
  }

 public:
  // Generate all positions that can reach position of board in one ply.
  std::vector<Board> preceding_positions(Board const& board);
};
