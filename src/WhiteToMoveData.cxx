#include "sys.h"
#include "WhiteToMoveData.h"
#include "BlackToMoveData.h"
#include "debug.h"

void WhiteToMoveData::add_edges(white_to_move_nodes_type::iterator const& current_board, black_to_move_nodes_type::iterator const& succeeding_board)
{
//  DoutEntering(dc::notice, "WhiteToMoveData::add_edges(" << *current_board << ", " << *succeeding_board << ")");

  // current_board must point the map containing this Board/WhiteToMoveData pair.
  ASSERT(&current_board->second == this);
  // Don't add the same edge twice.
  ASSERT(std::find(child_positions_.begin(), child_positions_.end(), succeeding_board) == child_positions_.end());
  child_positions_.push_back(succeeding_board);
  // Make sure we didn't add this edge before.
  ASSERT(std::find(succeeding_board->second.parent_positions_.begin(), succeeding_board->second.parent_positions_.end(), current_board) == succeeding_board->second.parent_positions_.end());
  succeeding_board->second.parent_positions_.push_back(current_board);
}

void WhiteToMoveData::set_maximum_ply_on_parents(int max_ply)
{
  for (black_to_move_nodes_type::iterator parent_position : parent_positions_)
  {
    BlackToMoveData& data = parent_position->second;
    int ply = data.ply();
    if (ply == -1 || max_ply < ply)
      data.set_mate_in_ply(max_ply);
  }
}

#ifdef CWDEBUG
void WhiteToMoveData::print_on(std::ostream& os) const
{
  Classification::print_on(os);
}
#endif
