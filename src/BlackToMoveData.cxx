#include "sys.h"
#include "BlackToMoveData.h"
#include "WhiteToMoveData.h"
#include "debug.h"

#if CW_DEBUG
#include "Graph.h"
#endif

void BlackToMoveData::add_edges(black_to_move_nodes_type::iterator const& current_data, white_to_move_nodes_type::iterator const& succeeding_data COMMA_DEBUG_ONLY(Graph const& graph))
{
//  DoutEntering(dc::notice, "BlackToMoveData::add_edges(" <<
//      *current_data << "[" << Board{graph.get_index(current_data)} << "], " <<
//      *succeeding_data << "[" << Board{graph.get_index(succeeding_data)} << "])");

  // current_data must point the map containing this Board/BlackToMoveData pair.
  ASSERT(&*current_data == this);
  // Don't add the same edge twice.
  ASSERT(std::find(child_positions_.begin(), child_positions_.end(), succeeding_data) == child_positions_.end());
  child_positions_.push_back(succeeding_data);
  // Make sure we didn't add this edge before.
  ASSERT(std::find(succeeding_data->parent_positions_.begin(), succeeding_data->parent_positions_.end(), current_data) ==
      succeeding_data->parent_positions_.end());
  succeeding_data->parent_positions_.push_back(current_data);
}

void BlackToMoveData::set_maximum_ply_on_parents(std::vector<white_to_move_nodes_type::iterator>& parents)
{
  // Only call set_maximum_ply_on_parents on a position that already has its `mate_in_moves_` detemined.
  ASSERT(mate_in_moves_ != -1);
  // Any parent position can not be mate in more than `mate_in_moves_ + 1` ply, because in the parent
  // position it is white to move and white would pick the move that leads to mate in the least number
  // of moves.
  int const max_ply = mate_in_moves_ + 1;
  for (white_to_move_nodes_type::iterator parent_data : parent_positions_)
  {
    WhiteToMoveData& data = *parent_data;
    int parent_ply = data.ply();
    // We `set_mate_in_ply` for incremental ply, starting with 0.
    // Therefore it can't happen that a parent_ply that is larger than max_ply is already set.
    ASSERT(parent_ply <= max_ply);
    // If this parent didn't have its number of ply determined yet, it must be mate in `max_ply`, see above.
    if (parent_ply == -1)
    {
      data.set_mate_in_ply(max_ply);
      parents.push_back(parent_data);
    }
  }
}

#ifdef CWDEBUG
void BlackToMoveData::print_on(std::ostream& os) const
{
  Classification::print_on(os);
}
#endif
