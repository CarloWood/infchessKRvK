#include "sys.h"
#include "Data.h"
#include "debug.h"

void Data::add_edge(nodes_type::const_iterator const& succeeding_board)
{
//  DoutEntering(dc::notice, "Data::add_edge(" << *succeeding_board << ")");

  // Don't add the same edge twice.
  ASSERT(std::find(possible_moves_.begin(), possible_moves_.end(), succeeding_board) == possible_moves_.end());
  possible_moves_.push_back(succeeding_board);
}

#ifdef CWDEBUG
void Data::print_on(std::ostream& os) const
{
  Classification::print_on(os);
}
#endif
