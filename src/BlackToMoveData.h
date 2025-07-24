#pragma once

#include "Board.h"
#include "Classification.h"
#include <vector>
#include <map>

class WhiteToMoveData;

class BlackToMoveData : public Classification
{
 public:
  // Must be the same type as Graph::black_to_move_nodes_type.
  using black_to_move_nodes_type = std::map<Board, BlackToMoveData, DistanceCompare>;
  // Must be the same type as Graph::white_to_move_nodes_type.
  using white_to_move_nodes_type = std::map<Board, WhiteToMoveData, DistanceCompare>;

 private:
  friend class WhiteToMoveData;
  std::vector<white_to_move_nodes_type::const_iterator> child_positions_;
  std::vector<white_to_move_nodes_type::iterator> parent_positions_;

 public:
  BlackToMoveData(Classification const& classification) : Classification(classification) { }
  std::vector<white_to_move_nodes_type::const_iterator> const& child_positions() const { return child_positions_; }

  void add_edges(black_to_move_nodes_type::iterator const& current_board, white_to_move_nodes_type::iterator const& succeeding_board);
  void set_maximum_ply_on_parents(int ply);

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};
