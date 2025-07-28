#include "sys.h"
#include "BlackToMoveData.h"
#include "WhiteToMoveData.h"
#include "debug.h"

namespace version1 {

void BlackToMoveData::add_edges(black_to_move_nodes_type::iterator const& current_board, white_to_move_nodes_type::iterator const& succeeding_board)
{
//  DoutEntering(dc::notice, "BlackToMoveData::add_edges(" << *current_board << ", " << *succeeding_board << ")");

  // current_board must point the map containing this Board/BlackToMoveData pair.
  ASSERT(&current_board->second == this);
  // Don't add the same edge twice.
  ASSERT(std::find(child_positions_.begin(), child_positions_.end(), succeeding_board) == child_positions_.end());
  child_positions_.push_back(succeeding_board);
  // Make sure we didn't add this edge before.
  ASSERT(std::find(succeeding_board->second.parent_positions_.begin(), succeeding_board->second.parent_positions_.end(), current_board) == succeeding_board->second.parent_positions_.end());
  succeeding_board->second.parent_positions_.push_back(current_board);
}

void BlackToMoveData::set_maximum_ply_on_parents(std::vector<white_to_move_nodes_type::iterator>& parents)
{
  // Only call set_maximum_ply_on_parents on a position that already has its `mate_in_moves_` detemined.
  ASSERT(mate_in_moves_ != -1);
  // Any parent position can not be mate in more than `mate_in_moves_ + 1` ply, because in the parent
  // position it is white to move and white would pick the move that leads to mate in the least number
  // of moves.
  int const max_ply = mate_in_moves_ + 1;
  for (white_to_move_nodes_type::iterator parent_position : parent_positions_)
  {
    WhiteToMoveData& data = parent_position->second;
    int parent_ply = data.ply();
    // We `set_mate_in_ply` for incremental ply, starting with 0.
    // Therefore it can't happen that a parent_ply that is larger than max_ply is already set.
    ASSERT(parent_ply <= max_ply);
    // If this parent didn't have its number of ply determined yet, it must be mate in `max_ply`, see above.
    if (parent_ply == -1)
    {
      data.set_mate_in_ply(max_ply);
      parents.push_back(parent_position);
    }
  }
}

#ifdef CWDEBUG
void BlackToMoveData::print_on(std::ostream& os) const
{
  Classification::print_on(os);
}
#endif

} // namespace version1
