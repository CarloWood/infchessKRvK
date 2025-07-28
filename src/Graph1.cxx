#include "sys.h"
#include "Graph.h"
#include "debug.h"

namespace version1 {

void Graph::classify()
{
  // Generate all possible positions.
  for (int bk_x = 0; bk_x < Board::board_size; ++bk_x)
  {
    for (int bk_y = 0; bk_y < Board::board_size; ++bk_y)
    {
      for (int wk_x = 0; wk_x < Board::board_size; ++wk_x)
      {
        for (int wk_y = 0; wk_y < Board::board_size; ++wk_y)
        {
          for (int wr_x = 0; wr_x < Board::board_size; ++wr_x)
          {
            for (int wr_y = 0; wr_y < Board::board_size; ++wr_y)
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
  generate_edges_with_color<black>();
  generate_edges_with_color<white>();
}

} // namespace version1

bool operator==(version0::Graph const& lhs, version1::Graph const& rhs)
{
  ASSERT(lhs.board_size() == version1::Graph::board_size);

  // Run over all positions in version0.
  version0::Graph::black_to_move_nodes_type const& black_to_move_map0 = lhs.black_to_move_map();
  version0::Graph::white_to_move_nodes_type const& white_to_move_map0 = lhs.white_to_move_map();
  for (auto&& board_data0 : black_to_move_map0)
  {
    version0::Board const& board0 = board_data0.first;
    version0::BlackToMoveData const& data0 = board_data0.second;

    version0::Square const& black_king0 = board0.black_king();
    version0::Square const& white_king0 = board0.white_king();
    version0::Square const& white_rook0 = board0.white_rook();

    using namespace coordinates;
    int const bkx = black_king0[x];
    int const bky = black_king0[y];
    int const wkx = white_king0[x];
    int const wky = white_king0[y];
    int const wrx = white_rook0[x];
    int const wry = white_rook0[y];
    Color const to_move = black;
    int const board_size = board0.board_size();

    version1::Square black_king1(bkx, bky);
    version1::Square white_king1(wkx, wky);
    version1::Square white_rook1(wrx, wry);
    version1::Board board1(black_king1, white_king1, white_rook1);
  }
  for (auto&& board_data0 : white_to_move_map0)
  {
    version0::Board const& board0 = board_data0.first;
    version0::WhiteToMoveData const& data0 = board_data0.second;

    version0::Square const& black_king0 = board0.black_king();
    version0::Square const& white_king0 = board0.white_king();
    version0::Square const& white_rook0 = board0.white_rook();

    using namespace coordinates;
    int bkx = black_king0[x];
    int bky = black_king0[y];
    int wkx = white_king0[x];
    int wky = white_king0[y];
    int wrx = white_rook0[x];
    int wry = white_rook0[y];
    Color to_move = white;
    int const board_size = board0.board_size();

    version1::Square black_king1(bkx, bky);
    version1::Square white_king1(wkx, wky);
    version1::Square white_rook1(wrx, wry);
    version1::Board board1(black_king1, white_king1, white_rook1);
  }

  return false;
}
