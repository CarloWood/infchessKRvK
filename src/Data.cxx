#include "sys.h"
#include "Data.h"
#include "debug.h"

void Data::add_edge(nodes_type::const_iterator const& next_board)
{
  // Don't add the same edge twice.
  ASSERT(std::find(possible_moves_.begin(), possible_moves_.end(), next_board) == possible_moves_.end());
  possible_moves_.push_back(next_board);
}
