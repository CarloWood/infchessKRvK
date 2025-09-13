#include "sys.h"
#include "Graph.h"
#include "Board.h"
#include "debug.h"
#include <algorithm>
#include <format>

void Graph::classify()
{
  // A dummy array.
  Board::neighbors_type neighbors;
  // Generate all possible positions.
  for (int bk_x = 0; bk_x < Size::board::x; ++bk_x)
  {
    for (int bk_y = 0; bk_y < Size::board::y; ++bk_y)
    {
      for (int wk_x = 0; wk_x < Size::board::x; ++wk_x)
      {
        for (int wk_y = 0; wk_y < Size::board::y; ++wk_y)
        {
          for (int wr_x = 0; wr_x < Size::board::x; ++wr_x)
          {
            for (int wr_y = 0; wr_y < Size::board::y; ++wr_y)
            {
              for (int color = 0; color < 2; ++color)
              {
                //FIXME: remove this.
                // We test one specific case only.
                bk_x = 0;
                bk_y = 0;
                wk_x = 0;
                wk_y = 2;
                wr_x = 0;
                wr_y = 1;
                color = 0;

                BlackKingSquare const black_king{bk_x, bk_y};
                WhiteKingSquare const white_king{wk_x, wk_y};
                WhiteRookSquare const white_rook{wr_x, wr_y};
                Color const to_move(static_cast<color_type>(color));

                Board const pos(black_king, white_king, white_rook);

                //FIXME: remove this.
                Dout(dc::notice, "Board under test:");
                pos.debug_utf8art(DEBUGCHANNELS::dc::notice);

                if (pos.determine_legal(to_move))
                {
                  Info& info = (to_move == black) ? get_info<black>(pos) : get_info<white>(pos);
                  Classification& classification = info.classification();
                  ASSERT(!classification.is_legal());
                  classification.determine(pos, to_move);

                  if (!classification.is_draw())
                  {
                    if (to_move == black)
                      info.set_number_of_children(pos.generate_neighbors<Board::children, black>(neighbors));
                    else
                      info.set_number_of_children(pos.generate_neighbors<Board::children, white>(neighbors));
                  }
                }

                //FIXME: remove this.
                // We are only testing a single board.
                return;
              }
            }
          }
        }
      }
    }
  }
}

//static
std::filesystem::path Graph::data_directory(std::filesystem::path const& prefix_directory)
{
  return prefix_directory /
         std::format("board{}x{}", Size::board_size_x, Size::board_size_y) /
         std::format("partition{}x{}", Size::Px, Size::Py);
}
