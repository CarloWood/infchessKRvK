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
  std::vector<nodes_type::const_iterator> possible_moves_;

 public:
  Data(Classification const& classification) : Classification(classification) { }
  std::vector<nodes_type::const_iterator> const& possible_moves() const { return possible_moves_; }

  void add_edge(nodes_type::const_iterator const& succeeding_board);

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};
