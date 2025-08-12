#pragma once

#include "Board.h"
#include "utils/has_print_on.h"
#include "utils/Vector.h"
#include <limits>

// This class defines a print_on method.
using utils::has_print_on::operator<<;

// Representation of a single `Info` object.
// The Board and whose move it is (the chess position) is defined by the context in which this object is being used.
class Info
{
 public:
  // Storing a ply.
  static constexpr unsigned int max_ply_estimate = 255;
  static constexpr int ply_bits = utils::ceil_log2(max_ply_estimate);
  using ply_type = uint_type<ply_bits>;
  static constexpr ply_type unknown_ply = std::numeric_limits<ply_type>::max();

  // Storing the number of children (or parents) of a given node.
  static constexpr int max_degree_bits = utils::ceil_log2(Board::max_degree);
  using degree_type = uint_type<max_degree_bits>;

  // The vector type used to store all Info objects (members of Graph).
  using info_nodes_type = utils::Vector<Info, InfoIndex>;

 private:
  ply_type mate_in_moves_;                      // Mate follows after `mate_in_moves_` ply.
  Classification classification_;               // The classification of this position.
  // The following are only valid if this position is legal.
  degree_type number_of_children_;              // The number of (legal) positions that can be reached from this position.
  degree_type number_of_visited_children_;      // The number of children that visited this parent, during generation of the graph.

 public:
  // Accessors.
  Classification& classification() { return classification_; }
  Classification const& classification() const { return classification_; }
  int ply() const { return mate_in_moves_; }
  degree_type number_of_children() const { return number_of_children_; }
  degree_type number_of_visited_children() const { return number_of_visited_children_; }

  // Given that black is to move, set the mate_in_moves_ value on each of the parent positions.
  void black_to_move_set_maximum_ply_on_parents(info_nodes_type::index_type current, info_nodes_type& infos, std::vector<Board>& parents);

 public:
  // Set in how many ply this position is mate.
  void set_mate_in_ply(int ply)
  {
    ASSERT(ply >= 0);
    // If it is a draw, then it isn't mate in `ply` moves; so why is this function being called?
    ASSERT(!classification_.is_draw());
    // If it mate then `ply` must be zero.
    ASSERT(classification_.is_mate() == (ply == 0));
    mate_in_moves_ = ply;
  }

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};
