#pragma once

#include "Board1.h"
#include "Classification.h"
#include <vector>
#include <map>

namespace version1 {

class BlackToMoveData;
#if CW_DEBUG
class Graph;
#endif

class WhiteToMoveData : public Classification
{
 public:
  // These must be the same types as in BlackToMoveData.
  using black_to_move_nodes_type = std::vector<BlackToMoveData>;
  using white_to_move_nodes_type = std::vector<WhiteToMoveData>;

 private:
  friend class BlackToMoveData;
  std::vector<black_to_move_nodes_type::const_iterator> child_positions_;       // All child (succeeding) positions.
  std::vector<black_to_move_nodes_type::iterator> parent_positions_;            // All parent (preceeding) positions.

 public:
  std::vector<black_to_move_nodes_type::const_iterator> const& child_positions() const { return child_positions_; }
  std::vector<black_to_move_nodes_type::iterator> const& parent_positions() const { return parent_positions_; }

  void add_edges(white_to_move_nodes_type::iterator current_board, black_to_move_nodes_type::iterator succeeding_board
      COMMA_DEBUG_ONLY(Graph const& graph));
  void set_minimum_ply_on_parents(std::vector<black_to_move_nodes_type::iterator>& parents);

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};

} // namespace version1
