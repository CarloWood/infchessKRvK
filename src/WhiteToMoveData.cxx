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

void WhiteToMoveData::set_minimum_ply_on_parents(std::vector<black_to_move_nodes_type::iterator>& parents)
{
  DoutEntering(dc::notice, "WhiteToMoveData::set_minimum_ply_on_parents(" << min_ply << ", ...)");

  // Only call set_minimum_ply_on_parents on a position that already has its `mate_in_moves_` detemined.
  ASSERT(mate_in_moves_ != -1);
  // Any parent position can not be mate in less than `mate_in_moves_ + 1` ply, because in the parent
  // position it is black to move and black would pick the move that leads to mate in the largest number
  // of moves.
  int const min_ply = mate_in_moves_ + 1;
  for (black_to_move_nodes_type::iterator parent_position : parent_positions_)
  {
    BlackToMoveData& black_to_move_data = parent_position->second;

    if (black_to_move_data.is_draw())
      continue;

    // Call set_minimum_ply_on_parents exactly once for each position (where white is to move).
    // In that case, the mate_in_moves_ member is only set when the last child has set_minimum_ply_on_parents called.
    ASSERT(black_to_move_data.ply() == -1);

    // Inform parent that another child has its mate_in_moves_ set. This will append the parent to parents
    // if the parent is now known to be mate in `min_ply` moves because this was its last child.
    if (black_to_move_data.increment_processed_children())      // Was this the last child?
    {
      parent_position->second.set_mate_in_ply(min_ply);
      parents.push_back(parent_position);
    }
  }
}

#ifdef CWDEBUG
void WhiteToMoveData::print_on(std::ostream& os) const
{
  Classification::print_on(os);
}
#endif
