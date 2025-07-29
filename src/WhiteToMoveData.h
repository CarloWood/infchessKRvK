#pragma once

#include "Board0.h"
#include "Board1.h"
#include "Classification.h"
#include <vector>
#include <map>

namespace version0 {

class BlackToMoveData;

class WhiteToMoveData : public Classification
{
 public:
  // These must be the same types as in BlackToMoveData.
  using black_to_move_nodes_type = std::map<Board, BlackToMoveData, DistanceCompare>;
  using white_to_move_nodes_type = std::map<Board, WhiteToMoveData, DistanceCompare>;

 private:
  friend class BlackToMoveData;
  std::vector<black_to_move_nodes_type::const_iterator> child_positions_;       // All child (succeeding) positions.
  std::vector<black_to_move_nodes_type::iterator> parent_positions_;            // All parent (preceeding) positions.

 public:
  std::vector<black_to_move_nodes_type::const_iterator> const& child_positions() const { return child_positions_; }
  std::vector<black_to_move_nodes_type::iterator> const& parent_positions() const { return parent_positions_; }

  void add_edges(white_to_move_nodes_type::iterator const& current_board, black_to_move_nodes_type::iterator const& succeeding_board);
  void set_minimum_ply_on_parents(std::vector<black_to_move_nodes_type::iterator>& parents);

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};

} // namespace version0

namespace version1 {

class BlackToMoveData;

class WhiteToMoveData : public Classification
{
 public:
  // These must be the same types as in BlackToMoveData.
  using black_to_move_nodes_type = std::map<Board, BlackToMoveData, DistanceCompare>;
  using white_to_move_nodes_type = std::map<Board, WhiteToMoveData, DistanceCompare>;

 private:
  friend class BlackToMoveData;
  std::vector<black_to_move_nodes_type::const_iterator> child_positions_;       // All child (succeeding) positions.
  std::vector<black_to_move_nodes_type::iterator> parent_positions_;            // All parent (preceeding) positions.

 public:
  std::vector<black_to_move_nodes_type::const_iterator> const& child_positions() const { return child_positions_; }
  std::vector<black_to_move_nodes_type::iterator> const& parent_positions() const { return parent_positions_; }

  void add_edges(white_to_move_nodes_type::iterator const& current_board, black_to_move_nodes_type::iterator const& succeeding_board);
  void set_minimum_ply_on_parents(std::vector<black_to_move_nodes_type::iterator>& parents);

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};

} // namespace version1
