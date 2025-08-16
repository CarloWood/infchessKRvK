#include "sys.h"
#include "Graph.h"
#include "Square.h"
#include "debug.h"
#include <algorithm>

void Graph::classify()
{
  // Generate all possible positions.
  for (int bk_x = 0; bk_x < Board::board_size_x; ++bk_x)
  {
    for (int bk_y = 0; bk_y < Board::board_size_y; ++bk_y)
    {
      for (int wk_x = 0; wk_x < Board::board_size_x; ++wk_x)
      {
        for (int wk_y = 0; wk_y < Board::board_size_y; ++wk_y)
        {
          for (int wr_x = 0; wr_x < Board::board_size_x; ++wr_x)
          {
            for (int wr_y = 0; wr_y < Board::board_size_y; ++wr_y)
            {
              for (int color = 0; color < 2; ++color)
              {
                Square black_king{bk_x, bk_y};
                Square white_king{wk_x, wk_y};
                Square white_rook{wr_x, wr_y};
                Color to_move(static_cast<color_type>(color));

                Board pos(black_king, white_king, white_rook);

                if (pos.determine_legal(to_move))
                {
                  Classification* classification;
                  size_t index = pos.as_index();
                  if (to_move == black)
                  {
                    ASSERT(!black_to_move_[index].is_legal());
                    BlackToMoveData& data = black_to_move_[index];
                    classification = &data;
                  }
                  else
                  {
                    ASSERT(!white_to_move_[index].is_legal());
                    WhiteToMoveData& data = white_to_move_[index];
                    classification = &data;
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
//  DoutEntering(dc::notice, "Graph::generate_edges()");
  generate_edges_with_color<black>();
  generate_edges_with_color<white>();
}

void Graph::write_to(std::ostream& os) const
{
  // Generate all possible positions.
  for (int color = 0; color < 2; ++color)
  {
    for (int bk_y = 0; bk_y < Board::board_size_y; ++bk_y)
    {
      for (int bk_x = 0; bk_x < Board::board_size_x; ++bk_x)
      {
        for (int wk_y = 0; wk_y < Board::board_size_y; ++wk_y)
        {
          for (int wk_x = 0; wk_x < Board::board_size_x; ++wk_x)
          {
            for (int wr_y = 0; wr_y < Board::board_size_y; ++wr_y)
            {
              for (int wr_x = 0; wr_x < Board::board_size_x; ++wr_x)
              {
                Square black_king{bk_x, bk_y};
                Square white_king{wk_x, wk_y};
                Square white_rook{wr_x, wr_y};
                Color to_move(static_cast<color_type>(color));

                Board pos(black_king, white_king, white_rook);

                Classification const* classification;
                size_t index = pos.as_index();
                if (to_move == black)
                {
                  BlackToMoveData const& data = black_to_move_[index];
                  classification = &data;
                }
                else
                {
                  WhiteToMoveData const& data = white_to_move_[index];
                  classification = &data;
                }
                classification->write_to(os);
              }
            }
          }
        }
      }
    }
  }
}

void Graph::read_from(std::istream& is)
{
  // Generate all possible positions.
  for (int color = 0; color < 2; ++color)
  {
    for (int bk_y = 0; bk_y < Board::board_size_y; ++bk_y)
    {
      for (int bk_x = 0; bk_x < Board::board_size_x; ++bk_x)
      {
        for (int wk_y = 0; wk_y < Board::board_size_y; ++wk_y)
        {
          for (int wk_x = 0; wk_x < Board::board_size_x; ++wk_x)
          {
            for (int wr_y = 0; wr_y < Board::board_size_y; ++wr_y)
            {
              for (int wr_x = 0; wr_x < Board::board_size_x; ++wr_x)
              {
                Square black_king{bk_x, bk_y};
                Square white_king{wk_x, wk_y};
                Square white_rook{wr_x, wr_y};
                Color to_move(static_cast<color_type>(color));

                Board pos(black_king, white_king, white_rook);

                Classification* classification;
                size_t index = pos.as_index();
                if (to_move == black)
                {
                  BlackToMoveData& data = black_to_move_[index];
                  classification = &data;
                }
                else
                {
                  WhiteToMoveData& data = white_to_move_[index];
                  classification = &data;
                }
                classification->read_from(is);
              }
            }
          }
        }
      }
    }
  }
}
