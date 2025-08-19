#include "sys.h"
#include "Graph.h"
#include "Board.h"
#include "debug.h"
#include <algorithm>

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
                BlackKingSquare const black_king{bk_x, bk_y};
                WhiteKingSquare const white_king{wk_x, wk_y};
                WhiteRookSquare const white_rook{wr_x, wr_y};
                Color const to_move(static_cast<color_type>(color));

                Board const pos(black_king, white_king, white_rook);

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
              }
            }
          }
        }
      }
    }
  }
}

void Graph::write_to(std::ostream& os) const
{
  for (int bky = 0; bky < Size::board::y; ++bky)
    for (int bkx = 0; bkx < Size::board::x; ++bkx)
      for (int wky = 0; wky < Size::board::y; ++wky)
        for (int wkx = 0; wkx < Size::board::x; ++wkx)
          for (int wry = 0; wry < 32 /*Size::board::y*/; ++wry)
            for (int wrx = 0; wrx < 32 /*Size::board::x*/; ++wrx)
            {
              if (utils::is_between_le_lt(0, wrx, (int)Size::board::x) && utils::is_between_le_lt(0, wry, (int)Size::board::y))
              {
                Board board({bkx, bky}, {wkx, wky}, {wrx, wry});
                Info const& info = get_info<black>(board);
                info.classification().write_to(os);
              }
              else
                Classification{}.write_to(os);
            }
  for (int bky = 0; bky < Size::board::y; ++bky)
    for (int bkx = 0; bkx < Size::board::x; ++bkx)
      for (int wky = 0; wky < Size::board::y; ++wky)
        for (int wkx = 0; wkx < Size::board::x; ++wkx)
          for (int wry = 0; wry < 32 /*Size::board::y*/; ++wry)
            for (int wrx = 0; wrx < 32 /*Size::board::x*/; ++wrx)
            {
              if (utils::is_between_le_lt(0, wrx, (int)Size::board::x) && utils::is_between_le_lt(0, wry, (int)Size::board::y))
              {
                Board board({bkx, bky}, {wkx, wky}, {wrx, wry});
                Info const& info = get_info<white>(board);
                info.classification().write_to(os);
              }
              else
                Classification{}.write_to(os);
            }
}

void Graph::read_from(std::istream& is)
{
  for (int bky = 0; bky < Size::board::y; ++bky)
    for (int bkx = 0; bkx < Size::board::x; ++bkx)
      for (int wky = 0; wky < Size::board::y; ++wky)
        for (int wkx = 0; wkx < Size::board::x; ++wkx)
          for (int wry = 0; wry < 32 /*Size::board::y*/; ++wry)
            for (int wrx = 0; wrx < 32 /*Size::board::x*/; ++wrx)
            {
              if (utils::is_between_le_lt(0, wrx, (int)Size::board::x) && utils::is_between_le_lt(0, wry, (int)Size::board::y))
              {
                Board board({bkx, bky}, {wkx, wky}, {wrx, wry});
                Info& info = get_info<black>(board);
                info.classification().read_from(is);
              }
              else
              {
                Info dummy;
                dummy.classification().read_from(is);
              }
            }
  for (int bky = 0; bky < Size::board::y; ++bky)
    for (int bkx = 0; bkx < Size::board::x; ++bkx)
      for (int wky = 0; wky < Size::board::y; ++wky)
        for (int wkx = 0; wkx < Size::board::x; ++wkx)
          for (int wry = 0; wry < 32 /*Size::board::y*/; ++wry)
            for (int wrx = 0; wrx < 32 /*Size::board::x*/; ++wrx)
            {
              if (utils::is_between_le_lt(0, wrx, (int)Size::board::x) && utils::is_between_le_lt(0, wry, (int)Size::board::y))
              {
                Board board({bkx, bky}, {wkx, wky}, {wrx, wry});
                Info& info = get_info<white>(board);
                info.classification().read_from(is);
              }
              else
              {
                Info dummy;
                dummy.classification().read_from(is);
              }
            }
}
