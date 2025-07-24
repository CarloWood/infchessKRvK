#pragma once

#include "Board.h"
#include "Classification.h"
#include <vector>
#include <map>

class Data : public Classification
{
 public:
  // Must be the same type as Graph::nodes_type.
  using nodes_type = std::map<Board, Data, DistanceCompare>;

 private:
  std::vector<nodes_type::const_iterator> child_positions_;
  std::vector<nodes_type::iterator> parent_positions_;

 public:
  Data(Classification const& classification) : Classification(classification) { }
  std::vector<nodes_type::const_iterator> const& child_positions() const { return child_positions_; }

  void add_edges(nodes_type::iterator const& current_board, nodes_type::iterator const& succeeding_board);
  void set_maximum_ply_on_parents(int ply);

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};
