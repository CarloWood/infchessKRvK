#include "sys.h"
#include "Info.h"
#include "Graph.h"
#include "utils/endian.h"
#include "debug.h"

void Info::black_to_move_set_maximum_ply_on_parents(Board const current_board, Graph& graph, std::vector<Board>& parents_out)
{
  DoutEntering(dc::notice, "Info::black_to_move_set_maximum_ply_on_parents(" << current_board << ", graph, parents_out)");

  // Only call black_to_move_set_maximum_ply_on_parents on a position that already has its `mate_in_ply_` determined.
  ASSERT(classification().ply() != Classification::unknown_ply);
  // No parent position can be mate in more than `mate_in_ply_ + 1` ply, because in the parent
  // position it is white to move and white would pick the move that leads to mate in the least
  // number of moves.
  Classification::ply_type const max_ply = classification().ply() + 1;
  // This number of ply plus one must fit in a ply_type.
  ASSERT(max_ply < Classification::max_encoded_ply);
  // Generate all parent positions.
  Board::neighbors_type parents;
  int number_of_parents = current_board.generate_neighbors<Board::parents, white>(parents);
#ifdef CWDEBUG
  bool oops = false;
  for (int i = 0; i < number_of_parents; ++i)
  {
    Board const& parent = parents[i];
    Info const& parent_info = graph.get_info<white>(parent);
    if (!parent_info.classification().is_legal())
    {
      oops = true;
      break;
    }
  }
  if (oops)
  {
    std::cout << "Called generate_neighbors<Board::parents, white> on:\n";
    current_board.utf8art(std::cout, black);
    std::cout << "Returned number of parents: " << number_of_parents << std::endl;
    for (int i = 0; i < number_of_parents; ++i)
    {
      Board const& parent = parents[i];
      Info const& parent_info = graph.get_info<white>(parent);
      if (!parent_info.classification().is_legal())
        std::cout << "ILLEGAL Parent " << i << ":\n";
      else
        std::cout << "Parent " << i << ":\n";
      parent.utf8art(std::cout, white);
    }
  }
#endif
  // Run over all parent positions.
  for (int i = 0; i < number_of_parents; ++i)
  {
    Board const& parent = parents[i];
    Info& parent_info = graph.get_info<white>(parent);
    // All returned parent positions should be legal.
    ASSERT(parent_info.classification().is_legal());
    int parent_ply = parent_info.classification().ply();
    // If this parent didn't have its number of ply determined yet, it must be mate in `max_ply`, see above.
    if (parent_ply == Classification::unknown_ply)
    {
      parent_info.classification().set_mate_in_ply(max_ply);
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

void Info::white_to_move_set_minimum_ply_on_parents(
    Board const current_board, Graph& graph, std::vector<Board>& parents_out)
{
  DoutEntering(dc::notice, "Info::white_to_move_set_minimum_ply_on_parents(" << current_board << ", graph, parents_out)");

  // Only call set_minimum_ply_on_parents on a position that already has its `mate_in_ply_` detemined.
  ASSERT(classification().ply() != Classification::unknown_ply);
  // No parent position can be mate in less than `mate_in_ply_ + 1` ply, because in the parent position
  // it is black to move and black would pick the move that leads to mate in the largest number of moves.
  int const min_ply = classification().ply() + 1;
  // This number of ply plus one must fit in a ply_type.
  ASSERT(min_ply < Classification::max_encoded_ply);
  Dout(dc::notice, "min_ply = " << min_ply);
  // Generate all parent positions.
  Board::neighbors_type parents;
  int number_of_parents = current_board.generate_neighbors<Board::parents, black>(parents);
  Dout(dc::notice, "number_of_parents = " << number_of_parents);
  // Run over all parent positions.
  for (int i = 0; i < number_of_parents; ++i)
  {
    Board const& parent = parents[i];
    Dout(dc::notice, "  parent " << i << " = " << parent);
    auto [parent_info, parent_non_mapped_info] = graph.get_info_tuple<black>(parent);
    Dout(dc::notice, "    with info: " << parent_info);
    // All returned parent positions should be legal.
    ASSERT(parent_info.classification().is_legal());

    // If black already has a draw in this (parent) position then it will never do the move that ends up as the current position.
    if (parent_info.classification().is_draw())
      continue;

    // Call white_to_move_set_minimum_ply_on_parents exactly once for each position (where white is to move).
    // In that case, the mate_in_ply_ member is only set after the last child called white_to_move_set_minimum_ply_on_parents.
    ASSERT(parent_info.classification().ply() == Classification::unknown_ply);

    // Inform parent that another child has its mate_in_ply_ set.
    // Append the parent to parents_out if the parent is now known to be mate in `min_ply` moves because this was its last child.
    if (parent_non_mapped_info.increment_processed_children(parent_info.number_of_children()))  // Was this the last child?
    {
      parent_info.classification().set_mate_in_ply(min_ply);
      parents_out.push_back(parent);
    }
  }
}

#ifdef CWDEBUG
void Info::print_on(std::ostream& os) const
{
  os << '{';
  os << "classification:" << classification_ <<
      ", number_of_children:" << static_cast<uint32_t>(number_of_children_);
  os << '}';
}

void NonMappedInfo::print_on(std::ostream& os) const
{
  os << '{';
  os << "number_of_visited_children:" << static_cast<uint32_t>(number_of_visited_children_);
  os << '}';
}
#endif
