#include "sys.h"
#include "Graph.h"
#include "parse_move.h"
#include <bitset>
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

#if 0
  // Construct the initial graph with all positions that are already mate.
  auto start2 = std::chrono::high_resolution_clock::now();

  Graph graph2;

  int total_positions2;
  int draw_positions2 = 0;
  int black_in_check_positions2 = 0;
  int mate_positions2 = 0;
  int stalemate_positions2 = 0;

  // Generate all possible positions.
  graph2.classify();

  std::vector<Graph::info_nodes_type::iterator> already_mate2;

  // Black to move positions.
  {
    Graph::info_nodes_type& black_to_move = graph2.black_to_move();
    total_positions2 = 0;
    for (Graph::info_nodes_type::iterator current_info = black_to_move.begin();
        current_info != black_to_move.end(); ++current_info)
    {
      Info const& info = *current_info;
      Classification const& pc = info.classification();
      if (!pc.is_legal())
        continue;
      ++total_positions2;
      if (pc.is_draw())
        ++draw_positions2;
      if (pc.is_check())
        ++black_in_check_positions2;
      if (pc.is_mate())
      {
        ++mate_positions2;
        already_mate2.push_back(current_info);
      }
      if (pc.is_stalemate())
        ++stalemate_positions2;
    }
  }
  // White to move positions.
  {
    Graph::info_nodes_type& white_to_move = graph2.white_to_move();
    // Run over all legal positions in white_to_move_map.
    for (Graph::info_nodes_type::iterator current_info = white_to_move.begin();
        current_info != white_to_move.end(); ++current_info)
    {
      Info const& info = *current_info;
      Classification const& pc = info.classification();
      if (!pc.is_legal())
        continue;
      ++total_positions2;
      if (pc.is_draw())
        ++draw_positions2;
    }
  }

  std::cout << "Version 2:" << std::endl;
  std::cout << "Total legal positions: " << total_positions2 << std::endl;
  std::cout << "Draw positions: " << draw_positions2 << std::endl;
  std::cout << "Black in check positions: " << black_in_check_positions2 << std::endl;
  std::cout << "Mate positions: " << mate_positions2 << std::endl;
  std::cout << "Stalemate positions: " << stalemate_positions2 << std::endl;

  auto end2 = std::chrono::high_resolution_clock::now();
  auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2);
  std::cout << "Execution time: " << (duration2.count() / 1000000.0) << " seconds\n";

  {
    Board::FieldSpec fs = Board::field_spec<coordinates::x, Board::wr>();
    std::bitset<Board::available_bits> mask = fs.mask;
    std::bitset<Board::available_bits> limit = fs.limit;
    std::bitset<Board::available_bits> stride = fs.stride;
    std::cout << "x coordinate of white rook:\n";
    std::cout << "mask  : " << mask << '\n';
    std::cout << "limit : " << limit << '\n';
    std::cout << "stride: " << stride << '\n';
  }
  {
    Board::FieldSpec fs = Board::field_spec<coordinates::y, Board::wr>();
    std::bitset<Board::available_bits> mask = fs.mask;
    std::bitset<Board::available_bits> limit = fs.limit;
    std::bitset<Board::available_bits> stride = fs.stride;
    std::cout << "y coordinate of white rook:\n";
    std::cout << "mask  : " << mask << '\n';
    std::cout << "limit : " << limit << '\n';
    std::cout << "stride: " << stride << '\n';
  }
  {
    Board::FieldSpec fs = Board::field_spec<coordinates::x, Board::wkbi>();
    std::bitset<Board::available_bits> mask = fs.mask;
    std::bitset<Board::available_bits> limit = fs.limit;
    std::bitset<Board::available_bits> stride = fs.stride;
    std::cout << "Block index (x) of the white king:\n";
    std::cout << "mask  : " << mask << '\n';
    std::cout << "limit : " << limit << '\n';
    std::cout << "stride: " << stride << '\n';
  }
  {
    Board::FieldSpec fs = Board::field_spec<coordinates::y, Board::wkbi>();
    std::bitset<Board::available_bits> mask = fs.mask;
    std::bitset<Board::available_bits> limit = fs.limit;
    std::bitset<Board::available_bits> stride = fs.stride;
    std::cout << "Block index (y) of the white king:\n";
    std::cout << "mask  : " << mask << '\n';
    std::cout << "limit : " << limit << '\n';
    std::cout << "stride: " << stride << '\n';
  }
  {
    Board::FieldSpec fs = Board::field_spec<coordinates::x, Board::wkbc>();
    std::bitset<Board::available_bits> mask = fs.mask;
    std::bitset<Board::available_bits> limit = fs.limit;
    std::bitset<Board::available_bits> stride = fs.stride;
    std::cout << "x coordinate of white king's block coordinates:\n";
    std::cout << "mask  : " << mask << '\n';
    std::cout << "limit : " << limit << '\n';
    std::cout << "stride: " << stride << '\n';
  }
  {
    Board::FieldSpec fs = Board::field_spec<coordinates::y, Board::wkbc>();
    std::bitset<Board::available_bits> mask = fs.mask;
    std::bitset<Board::available_bits> limit = fs.limit;
    std::bitset<Board::available_bits> stride = fs.stride;
    std::cout << "y coordinate of white king's block coordinates:\n";
    std::cout << "mask  : " << mask << '\n';
    std::cout << "limit : " << limit << '\n';
    std::cout << "stride: " << stride << '\n';
  }
  {
    Board::FieldSpec fs = Board::field_spec<coordinates::x, Board::bkbi>();
    std::bitset<Board::available_bits> mask = fs.mask;
    std::bitset<Board::available_bits> limit = fs.limit;
    std::bitset<Board::available_bits> stride = fs.stride;
    std::cout << "Block index (x) of the black king:\n";
    std::cout << "mask  : " << mask << '\n';
    std::cout << "limit : " << limit << '\n';
    std::cout << "stride: " << stride << '\n';
  }
  {
    Board::FieldSpec fs = Board::field_spec<coordinates::y, Board::bkbi>();
    std::bitset<Board::available_bits> mask = fs.mask;
    std::bitset<Board::available_bits> limit = fs.limit;
    std::bitset<Board::available_bits> stride = fs.stride;
    std::cout << "Block index (y) of the black king:\n";
    std::cout << "mask  : " << mask << '\n';
    std::cout << "limit : " << limit << '\n';
    std::cout << "stride: " << stride << '\n';
  }
  {
    Board::FieldSpec fs = Board::field_spec<coordinates::x, Board::bkbc>();
    std::bitset<Board::available_bits> mask = fs.mask;
    std::bitset<Board::available_bits> limit = fs.limit;
    std::bitset<Board::available_bits> stride = fs.stride;
    std::cout << "x coordinate of black king's block coordinates:\n";
    std::cout << "mask  : " << mask << '\n';
    std::cout << "limit : " << limit << '\n';
    std::cout << "stride: " << stride << '\n';
  }
  {
    Board::FieldSpec fs = Board::field_spec<coordinates::y, Board::bkbc>();
    std::bitset<Board::available_bits> mask = fs.mask;
    std::bitset<Board::available_bits> limit = fs.limit;
    std::bitset<Board::available_bits> stride = fs.stride;
    std::cout << "y coordinate of black king's block coordinates:\n";
    std::cout << "mask  : " << mask << '\n';
    std::cout << "limit : " << limit << '\n';
    std::cout << "stride: " << stride << '\n';
  }
#endif

#if 0
  // Board::inc_field<0, bkbi>() with this = [{black king:(10, 3), w
  Board board({0, 0}, {4, 20}, {0, 0});
  board.utf8art(std::cout, black);
  board.dec_field<coordinates::x, Board::wkbi>();
  board.utf8art(std::cout, black);

#elif 0
  Board board({0, 0}, {7, 10}, {7, 10});
  do
  {
    Board b1(board);
    Dout(dc::notice, "Copied board --> b1 : " << b1);
    do
    {
      Board b2(b1);
      Dout(dc::notice, "Copied b1 --> b2 : " << b2);
      do
      {
        Board b3(b2);
        Dout(dc::notice, "Copied b2 --> b3 : " << b3);
        b3.utf8art(std::cout, black);
        while (b3.inc_field<coordinates::x, Board::wkbc>())
          b3.utf8art(std::cout, black);
      }
      while (b2.inc_field<coordinates::x, Board::wkbi>());
    }
    while (b1.inc_field<coordinates::y, Board::wkbc>());
  }
  while (board.inc_field<coordinates::y, Board::wkbi>());
#else
  Board board({14, 20}, {0, 0}, {14, 20});
  do
  {
    Board b1(board);
    b1.utf8art(std::cout, white);
    while (b1.dec_field<coordinates::y, Board::wr>())
      b1.utf8art(std::cout, white);
  }
  while (board.dec_field<coordinates::x, Board::wr>());
#endif

  return 0;

#if 0
  Board board({4, 5}, {7, 3}, {0, 1});
  Color to_move(white);
  int ply = -1;
  for (;;)
  {
    std::cout << "\nCurrent position (" << to_move << " to move; mate in " << ply << " ply):\n";
    board.utf8art(std::cout, to_move);

    // Get a move from the user.
    for (;;)
    {
      auto [piece, x, y] = parse_move(to_move, board_size_x, board_size_y);
      Board new_board(board);
      if (to_move == white)
      {
        if (piece == 'R')
          new_board.set_white_rook_square({x, y});
        else
          new_board.set_white_king_square({x, y});
      }
      else if (piece == 'K')
        new_board.set_black_king_square({x, y});

      board = new_board;
      break;
    }

    to_move = to_move.opponent();
  }
#endif
}
