#include "sys.h"
#include "Board.h"
#include "Graph.h"
#include "utils/print_using.h"
#include <chrono>
#include <thread>
#include <iostream>
#include "debug.h"

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  // Construct the initial graph with all positions that are already mate.
  Graph graph;

  // Generate all positions that are mate in 1, 2, ..., `ply` ply.
  int ply = 2;
  Debug(libcwd::libcw_do.off());
  graph.generate(ply);
  Debug(libcwd::libcw_do.on());

  Dout(dc::notice, " .--Mate in " << ply << " ply positions:");
  {
    NAMESPACE_DEBUG::Mark __mark;
    auto const& mate_in_ply_positions = graph.mate_in_ply(ply);
    for (Graph::map_type::value_type const& board : mate_in_ply_positions)
    {
      Debug(board.first.debug_utf8art(dc::notice));
    }
  }
  Dout(dc::notice, " `--End of mate in " << ply << " ply positions.");
}
