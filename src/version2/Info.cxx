#include "sys.h"
#include "Info.h"

#ifdef CWDEBUG
void Info::print_on(std::ostream& os) const
{
  Classification classification_;                            // The classification of this position.
  // The following are only valid if this position is legal.
  Size::ply_type mate_in_moves_;                             // Mate follows after `mate_in_moves_` ply.
  Size::number_of_children_type number_of_children_;         // The number of (legal) positions that can be reached from this position.
  Size::number_of_children_type number_of_visited_children_; // The number of children that visited this parent, during generation of the graph.

  os << '{';
  os << "classification:" << classification_ <<
      ", mate_in_moves:" << mate_in_moves_ <<
      ", number_of_children:" << number_of_children_ <<
      ", number_of_visited_children:" << number_of_visited_children_;
  os << '}';
}
#endif
