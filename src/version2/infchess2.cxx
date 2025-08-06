#include "sys.h"
#include "WhiteRookSquare.h"
#include "WhiteKingSquare.h"
#include "BlackKingSquare.h"
#include "Info.h"
#include "debug.h"

int main()
{
  Debug(NAMESPACE_DEBUG::init());

  Dout(dc::notice, "sizeof(Info) = " << sizeof(Info));

  // Get the size of the board.
  int const board_size_x = Size::board::x;
  int const board_size_y = Size::board::y;
  Dout(dc::notice, "Board size: " << board_size_x << "x" << board_size_y);

  // Get the size of a block.
  int const block_size_x = Size::block::x;
  int const block_size_y = Size::block::y;
  Dout(dc::notice, "Block size: " << block_size_x << "x" << block_size_y);

  for (int x = 0; x < board_size_x; ++x)
    for (int y = 0; y < board_size_y; ++y)
    {
      WhiteRookSquare wr(x, y);
      ASSERT(wr.x_coord() == x && wr.y_coord() == y);
    }

  for (int x = 0; x < board_size_x; ++x)
    for (int y = 0; y < board_size_y; ++y)
    {
      WhiteKingSquare wk(x, y);
      ASSERT(wk.x_coord() == x && wk.y_coord() == y);
    }

  for (int x = 0; x < board_size_x; ++x)
    for (int y = 0; y < board_size_y; ++y)
    {
      BlackKingSquare bk(x, y);
      ASSERT(bk.x_coord() == x && bk.y_coord() == y);
    }

  Board board;
}
