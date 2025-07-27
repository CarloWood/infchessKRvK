#pragma once

#include "Board.h"
#include "Classification.h"
#include <vector>
#include <map>

namespace version0 {

class WhiteToMoveData;

class BlackToMoveData : public Classification
{
 public:
  // These must be the same types as in BlackToMoveData.
  using black_to_move_nodes_type = std::map<Board, BlackToMoveData, DistanceCompare>;
  using white_to_move_nodes_type = std::map<Board, WhiteToMoveData, DistanceCompare>;

 private:
  friend class WhiteToMoveData;
  std::vector<white_to_move_nodes_type::const_iterator> child_positions_;       // All child (succeeding) positions.
  std::vector<white_to_move_nodes_type::iterator> parent_positions_;            // All parent (preceeding) positions.
  int processed_children_{0};                                                   // Counts the number of child positions on which
                                                                                // `WhiteToMoveData::set_minimum_ply_on_parents` has been called.

 public:
  std::vector<white_to_move_nodes_type::const_iterator> const& child_positions() const { return child_positions_; }

  void add_edges(black_to_move_nodes_type::iterator const& current_board, white_to_move_nodes_type::iterator const& succeeding_board);
  void set_maximum_ply_on_parents(std::vector<white_to_move_nodes_type::iterator>& parents);

  // Returns true if this was the last child.
  bool increment_processed_children()
  {
    return ++processed_children_ == child_positions_.size();
  }

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};

} // namespace version0
