#include "sys.h"
#include "Graph.h"
#include "Square1.h"
#include "debug.h"
#include <algorithm>

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
//  DoutEntering(dc::notice, "version1::Graph::generate_edges()");
  generate_edges_with_color<black>();
  generate_edges_with_color<white>();
}

} // namespace version1

bool operator==(version0::Graph const& lhs, version1::Graph const& rhs)
{
  ASSERT(lhs.board_size() == version1::Board::board_size);

  // Get a reference to the containers with the positions of version1.
  version1::Graph::black_to_move_nodes_type const& black_to_move_map1 = rhs.black_to_move_map();
  version1::Graph::white_to_move_nodes_type const& white_to_move_map1 = rhs.white_to_move_map();
  // Run over all positions in version0.
  version0::Graph::black_to_move_nodes_type const& black_to_move_map0 = lhs.black_to_move_map();
  version0::Graph::white_to_move_nodes_type const& white_to_move_map0 = lhs.white_to_move_map();

  int total_boards = black_to_move_map0.size() + white_to_move_map0.size();
  int total_tested = 0;

  // Run over all positions with black to move.
  for (auto&& board_data0 : black_to_move_map0)
  {
    // Decompose each position into its components.
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

    // Reconstruct a version1::Board from this.
    version1::Square black_king1(bkx, bky);
    version1::Square white_king1(wkx, wky);
    version1::Square white_rook1(wrx, wry);
    version1::Board board1(black_king1, white_king1, white_rook1);

    // Get the corresponding version1 BlackToMoveData.
    version1::BlackToMoveData const& data1 = black_to_move_map1[board1.as_index()];
    // Every position that exists in the version0 map of positions with black to move
    // must also exist in the version1 map of positions with black to move.
    ASSERT(data1.is_legal());

    // Compare the classification with that of version0.
    ASSERT(
        data0.is_mate() == data1.is_mate() &&
        data0.is_stalemate() == data1.is_stalemate() &&
        data0.is_draw() == data1.is_draw() &&
        data0.is_check() == data1.is_check());

    // Make sure the number of parent and child positions are the same.
    ASSERT(data0.parent_positions().size() == data1.parent_positions().size());
    ASSERT(data0.child_positions().size() == data1.child_positions().size());

    // Run over all child positions of data0.
    for (auto&& child_board_data0 : data0.child_positions())
    {
      // Decompose each position into its components.
      version0::Board const& child_board0 = child_board_data0->first;
      version0::WhiteToMoveData const& child_data0 = child_board_data0->second;

      version0::Square const& child_black_king0 = child_board0.black_king();
      version0::Square const& child_white_king0 = child_board0.white_king();
      version0::Square const& child_white_rook0 = child_board0.white_rook();

      using namespace coordinates;
      int const bkx = child_black_king0[x];
      int const bky = child_black_king0[y];
      int const wkx = child_white_king0[x];
      int const wky = child_white_king0[y];
      int const wrx = child_white_rook0[x];
      int const wry = child_white_rook0[y];
      Color const child_to_move = white;

      // Reconstruct a version1::Board from this.
      version1::Square child_black_king1(bkx, bky);
      version1::Square child_white_king1(wkx, wky);
      version1::Square child_white_rook1(wrx, wry);
      version1::Board child_board1(child_black_king1, child_white_king1, child_white_rook1);

      // Get the index.
      size_t index = child_board1.as_index();
      // Get the data pointer to its map element.
      version1::WhiteToMoveData const* data_ptr = &rhs.white_to_move_map()[index];

      // Find this child board in the version1 list of child positions of the current board.
      auto iter = std::find_if(data1.child_positions().begin(), data1.child_positions().end(),
          [&](auto&& i){ return &*i == data_ptr; });
      // Every position that exists in the version0 list of child positions
      // must also exist in the version1 list of child positions.
      ASSERT(iter != data1.child_positions().end());
    }
    // Run over all parent positions of data0.
    for (auto&& parent_board_data0 : data0.parent_positions())
    {
      version0::Board const& parent_board0 = parent_board_data0->first;
      version0::WhiteToMoveData const& parent_data0 = parent_board_data0->second;

      version0::Square const& parent_black_king0 = parent_board0.black_king();
      version0::Square const& parent_white_king0 = parent_board0.white_king();
      version0::Square const& parent_white_rook0 = parent_board0.white_rook();

      using namespace coordinates;
      int const bkx = parent_black_king0[x];
      int const bky = parent_black_king0[y];
      int const wkx = parent_white_king0[x];
      int const wky = parent_white_king0[y];
      int const wrx = parent_white_rook0[x];
      int const wry = parent_white_rook0[y];
      Color const parent_to_move = white;

      version1::Square parent_black_king1(bkx, bky);
      version1::Square parent_white_king1(wkx, wky);
      version1::Square parent_white_rook1(wrx, wry);
      version1::Board parent_board1(parent_black_king1, parent_white_king1, parent_white_rook1);

      // Get the index.
      size_t index = parent_board1.as_index();
      // Get the data pointer to its map element.
      version1::WhiteToMoveData const* data_ptr = &rhs.white_to_move_map()[index];

      // Find this parent board in the version1 list of parent positions of the current board.
      auto iter = std::find_if(data1.parent_positions().begin(), data1.parent_positions().end(),
          [&](auto&& i){ return &*i == data_ptr; });
      // Every position that exists in the version0 list of parent positions
      // must also exist in the version1 list of parent positions.
      ASSERT(iter != data1.parent_positions().end());
    }

    //iter->first.utf8art(std::cout);

    // The number of ply till mate must be the same between version0 and version1.
    ASSERT(data0.ply() == data1.ply());

    ++total_tested;
  }
  // Run over all positions with white to move.
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

    // Get the corresponding version1 BlackToMoveData.
    version1::WhiteToMoveData const& data1 = white_to_move_map1[board1.as_index()];
    // Every position that exists in the version0 map of positions with black to move
    // must also exist in the version1 map of positions with black to move.
    ASSERT(data1.is_legal());

    // Compare the classification with that of version0.
    ASSERT(
        data0.is_mate() == data1.is_mate() &&
        data0.is_stalemate() == data1.is_stalemate() &&
        data0.is_draw() == data1.is_draw() &&
        data0.is_check() == data1.is_check());

    ASSERT(data0.child_positions().size() == data1.child_positions().size());
    ASSERT(data0.parent_positions().size() == data1.parent_positions().size());

    // Run over all child positions of data0.
    for (auto&& child_board_data0 : data0.child_positions())
    {
      version0::Board const& child_board0 = child_board_data0->first;
      version0::BlackToMoveData const& child_data0 = child_board_data0->second;

      version0::Square const& child_black_king0 = child_board0.black_king();
      version0::Square const& child_white_king0 = child_board0.white_king();
      version0::Square const& child_white_rook0 = child_board0.white_rook();

      using namespace coordinates;
      int const bkx = child_black_king0[x];
      int const bky = child_black_king0[y];
      int const wkx = child_white_king0[x];
      int const wky = child_white_king0[y];
      int const wrx = child_white_rook0[x];
      int const wry = child_white_rook0[y];
      Color const child_to_move = black;

      version1::Square child_black_king1(bkx, bky);
      version1::Square child_white_king1(wkx, wky);
      version1::Square child_white_rook1(wrx, wry);
      version1::Board child_board1(child_black_king1, child_white_king1, child_white_rook1);

      // Get the index.
      size_t index = child_board1.as_index();
      // Get the data pointer to its map element.
      version1::BlackToMoveData const* data_ptr = &rhs.black_to_move_map()[index];

      // Find this child board in the version1 list of child positions of the current board.
      auto iter = std::find_if(data1.child_positions().begin(), data1.child_positions().end(),
          [&](auto&& i){ return &*i == data_ptr; });
      // Every position that exists in the version0 list of child positions
      // must also exist in the version1 list of child positions.
      ASSERT(iter != data1.child_positions().end());
    }
    // Run over all parent positions of data0.
    for (auto&& parent_board_data0 : data0.parent_positions())
    {
      version0::Board const& parent_board0 = parent_board_data0->first;
      version0::BlackToMoveData const& parent_data0 = parent_board_data0->second;

      version0::Square const& parent_black_king0 = parent_board0.black_king();
      version0::Square const& parent_white_king0 = parent_board0.white_king();
      version0::Square const& parent_white_rook0 = parent_board0.white_rook();

      using namespace coordinates;
      int const bkx = parent_black_king0[x];
      int const bky = parent_black_king0[y];
      int const wkx = parent_white_king0[x];
      int const wky = parent_white_king0[y];
      int const wrx = parent_white_rook0[x];
      int const wry = parent_white_rook0[y];
      Color const parent_to_move = black;

      version1::Square parent_black_king1(bkx, bky);
      version1::Square parent_white_king1(wkx, wky);
      version1::Square parent_white_rook1(wrx, wry);
      version1::Board parent_board1(parent_black_king1, parent_white_king1, parent_white_rook1);

      // Get the index.
      size_t index = parent_board1.as_index();
      // Get the data pointer to its map element.
      version1::BlackToMoveData const* data_ptr = &rhs.black_to_move_map()[index];

      // Find this parent board in the version1 list of parent positions of the current board.
      auto iter = std::find_if(data1.parent_positions().begin(), data1.parent_positions().end(),
          [&](auto&& i){ return &*i == data_ptr; });
      // Every position that exists in the version0 list of parent positions
      // must also exist in the version1 list of parent positions.
      ASSERT(iter != data1.parent_positions().end());
    }

    //iter->first.utf8art(std::cout);

    // The number of ply till mate must be the same between version0 and version1.
    ASSERT(data0.ply() == data1.ply());

    ++total_tested;
  }

  ASSERT(total_tested == total_boards);
  return true;
}
