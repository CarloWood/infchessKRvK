#include "sys.h"
#include "Graph.h"
#include "debug.h"

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
channel_ct alternate("ALTERNATE");
channel_ct adjacent("ADJACENT");
channel_ct edge("EDGE");
NAMESPACE_DEBUG_CHANNELS_END
#endif

namespace version0 {

Graph::Graph(int board_size) : board_size_(board_size)
{
}

void Graph::classify()
{
  // Generate all possible positions.
  for (int bk_x = 0; bk_x < board_size_; ++bk_x)
  {
    for (int bk_y = 0; bk_y < board_size_; ++bk_y)
    {
      for (int wk_x = 0; wk_x < board_size_; ++wk_x)
      {
        for (int wk_y = 0; wk_y < board_size_; ++wk_y)
        {
          for (int wr_x = 0; wr_x < board_size_; ++wr_x)
          {
            for (int wr_y = 0; wr_y < board_size_; ++wr_y)
            {
              for (int color = 0; color < 2; ++color)
              {
                Square black_king{bk_x, bk_y};
                Square white_king{wk_x, wk_y};
                Square white_rook{wr_x, wr_y};
                Color to_move(static_cast<color_type>(color));

                Board pos(board_size_, black_king, white_king, white_rook);

                if (pos.determine_legal(to_move))
                {
                  Classification* classification;
                  if (to_move == black)
                  {
                    auto ibp = black_to_move_.try_emplace(pos);
                    ASSERT(ibp.second);
                    classification = &ibp.first->second;
                  }
                  else
                  {
                    auto ibp = white_to_move_.try_emplace(pos);
                    ASSERT(ibp.second);
                    classification = &ibp.first->second;
                  }
                  classification->determine(pos, to_move);
                }
              }
            }
          }
        }
      }
    }
  }
}

void Graph::generate_edges()
{
  //DoutEntering(dc::notice, "version0::Graph::generate_edges()");
  generate_edges_with_color<black>();
  generate_edges_with_color<white>();
}

} // namespace version0
