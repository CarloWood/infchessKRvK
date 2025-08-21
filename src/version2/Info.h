#pragma once

#include "PartitionElement.h"
#include "Board.h"
#include "Classification.h"
#include "utils/has_print_on.h"
#include "utils/Array.h"
#include <limits>
#include <cmath>

class Graph;

// This class defines a print_on method.
using utils::has_print_on::operator<<;

// Representation of a single `Info` object.
// The Board and whose move it is (the chess position) is defined by the context in which this object is being used.
class Info
{
 public:
  // Storing the number of children (or parents) of a given node.
  static constexpr int max_degree_bits = utils::log2(Board::max_degree) + 1;
  using degree_type = uint_type<max_degree_bits>;

  // The vector type used to store all Info objects (members of Graph).
  using nodes_type = utils::Array<Info, PartitionElement::number_of_elements, InfoIndex>;

 private:
  Classification classification_;               // The classification of this position.
  // The following are only valid if this position is legal.
  degree_type number_of_children_{};            // The number of (legal) positions that can be reached from this position.
  degree_type number_of_visited_children_{0};   // The number of children that visited this parent, during generation of the graph.

 public:
  // Accessors.
  Classification& classification() { return classification_; }
  Classification const& classification() const { return classification_; }
  degree_type number_of_children() const { return number_of_children_; }
  degree_type number_of_visited_children() const { return number_of_visited_children_; }

  // Given that black is to move, set the mate_in_ply_ value on each of the parent positions.
  void black_to_move_set_maximum_ply_on_parents(Board const current_board, Graph& graph, std::vector<Board>& parents_out);
  void white_to_move_set_minimum_ply_on_parents(Board const current_board, Graph& graph, std::vector<Board>& parents_out);

  void set_number_of_children(degree_type number_of_children)
  {
    // Call this function only once.
    ASSERT(number_of_children_ == 0);
    number_of_children_ = number_of_children;
  }

  // Returns true if this was the last child.
  bool increment_processed_children()
  {
    // Call set_number_of_children first.
    ASSERT(number_of_children_ > 0);
    // This should be called exactly once for each child position.
    ASSERT(number_of_visited_children_ < number_of_children_);
    return ++number_of_visited_children_ == number_of_children_;
  }

 public:
#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};
