#include "sys.h"
#include "Info.h"
#include "debug.h"

void Info::black_to_move_set_maximum_ply_on_parents(
    info_nodes_type::index_type current, info_nodes_type& parent_infos_out, std::vector<Board>& parents_out)
{
  // Only call black_to_move_set_maximum_ply_on_parents on a position that already has its `mate_in_moves_` determined.
  ASSERT(mate_in_moves_ != unknown_ply);
  // No parent position can be mate in more than `mate_in_moves_ + 1` ply, because in the parent
  // position it is white to move and white would pick the move that leads to mate in the least
  // number of moves.
  ply_type const max_ply = mate_in_moves_ + 1;
  // This number of ply doesn't fit in a ply_type.
  ASSERT(max_ply != unknown_ply);
  // Generate all parent positions.
  Board const current_board(current);
  Board::neighbors_type parents;
  int number_of_parents = current_board.generate_neighbors<Board::parents, white>(parents);
  // Run over all parent positions.
  for (int i = 0; i < number_of_parents; ++i)
  {
    Board const& parent = parents[i];
    Info& parent_info = parent_infos_out[parent.as_index()];
    int parent_ply = parent_info.ply();
    // If this parent didn't have its number of ply determined yet, it must be mate in `max_ply`, see above.
    if (parent_ply == unknown_ply)
    {
      parent_info.set_mate_in_ply(max_ply);
      parents_out.push_back(parent);
    }
    else
    {
      // We `set_mate_in_ply` for incremental ply, starting with 0.
      // Therefore it can't happen that a parent_ply that is larger than max_ply is already set.
      ASSERT(parent_ply <= max_ply);
    }
  }
}

#ifdef CWDEBUG
void Info::print_on(std::ostream& os) const
{
  Classification classification_;               // The classification of this position.
  // The following are only valid if this position is legal.
  ply_type mate_in_moves_;                      // Mate follows after `mate_in_moves_` ply.
  degree_type number_of_children_;              // The number of (legal) positions that can be reached from this position.
  degree_type number_of_visited_children_;      // The number of children that visited this parent, during generation of the graph.

  os << '{';
  os << "classification:" << classification_ <<
      ", mate_in_moves:" << mate_in_moves_ <<
      ", number_of_children:" << number_of_children_ <<
      ", number_of_visited_children:" << number_of_visited_children_;
  os << '}';
}
#endif
