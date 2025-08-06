#pragma once

#include "Size.h"
#include "utils/has_print_on.h"

// This class defines a print_on method.
using utils::has_print_on::operator<<;

// Representation of a single `Info` object.
// The Board and whose move it is (the chess position) is defined by the context in which this object is being used.
class Info
{
 private:
  Classification classification_;                            // The classification of this position.
  // The following are only valid if this position is legal.
  Size::ply_type mate_in_moves_;                             // Mate follows after `mate_in_moves_` ply.
  Size::number_of_children_type number_of_children_;         // The number of (legal) positions that can be reached from this position.
  Size::number_of_children_type number_of_visited_children_; // The number of children that visited this parent, during generation of the graph.

 public:
  // Accessors.
  Classification const& classification() const { return classification_; }
  int ply() const { return mate_in_moves_; }
  Size::number_of_children_type number_of_children() const { return number_of_children_; }
  Size::number_of_children_type number_of_visited_children() const { return number_of_visited_children_; }

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
