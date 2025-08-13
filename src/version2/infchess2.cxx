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

  std::vector<InfoIndex> already_mate2;

  // Black to move positions.
  {
    Graph::info_nodes_type& black_to_move = graph2.black_to_move();
    total_positions2 = 0;
    for (Graph::info_nodes_type::index_type current_info_index = black_to_move.ibegin();
        current_info_index != black_to_move.iend(); ++current_info_index)
    {
      Info const& info = black_to_move[current_info_index];
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
        already_mate2.push_back(current_info_index);
      }
      if (pc.is_stalemate())
        ++stalemate_positions2;
    }
  }
  // White to move positions.
  {
    Graph::info_nodes_type& white_to_move = graph2.white_to_move();
    // Run over all legal positions in white_to_move_map.
    for (Graph::info_nodes_type::index_type current_info_index = white_to_move.ibegin();
        current_info_index != white_to_move.iend(); ++current_info_index)
    {
      Info const& info = white_to_move[current_info_index];
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

  // Run over all positions that are already mate (as per the classification)
  // and mark all position that can reach those as mate in 1 ply.
  std::vector<Board> white_to_move_parents2;
  Graph::info_nodes_type& black_to_move = graph2.black_to_move();
  for (Info::info_nodes_type::index_type info_index : already_mate2)
  {
    Info& black_to_move_info = black_to_move[info_index];
    black_to_move_info.set_mate_in_ply(0);
    black_to_move_info.black_to_move_set_maximum_ply_on_parents(info_index, black_to_move, white_to_move_parents2);
  }

#if 1
  constexpr int center_x = Size::board::x / 3;
  constexpr int center_y = Size::board::y / 3;

  BlackKingSquare const bk_positions[] = {
    {0, 0},                                     // Corner
    {0, Size::board::y - 1},                    // Corner
    {Size::board::x - 1, 0},                    // Corner
    {Size::board::x - 1, Size::board::y - 1},   // Corner
    {0, center_y},                              // Edge (west)
    {Size::board::x - 1, center_y},             // Edge (east)
    {center_x, 0},                              // Edge (south)
    {center_x, Size::board::y - 1},             // Edge (north)
    {center_x, center_y}                        // Center (no edges)
  };

  struct Pos
  {
    int c;
    bool c_is_offset;

    int operator+(int c_) const
    {
      if (c_is_offset)
        return c + c_;
      return c;
    }

    bool operator==(Pos const& rhs) const
    {
      return c == rhs.c && c_is_offset == rhs.c_is_offset;
    }
  };
  struct PosOffset { Pos x; Pos y; };

  std::vector<PosOffset> wk_offsets;
  wk_offsets.emplace_back(Pos{2 * center_x, false}, Pos{2 * center_y, false});   // Far away (no influence).
  // Two squares away.
  for (int i = -2; i <= 2; ++i)
  {
    wk_offsets.emplace_back(Pos{i, true}, Pos{-2, true});
    wk_offsets.emplace_back(Pos{i, true}, Pos{2, true});
    if (i > -2 && i < 2)
    {
      wk_offsets.emplace_back(Pos{-2, true}, Pos{i, true});
      wk_offsets.emplace_back(Pos{2, true}, Pos{i, true});
    }
  }

  std::array<Pos, 5> wr_positions_x = {{{0, false}, {-1, true}, {0, true}, {1, true}, {Size::board::x - 1, false}}};
  std::array<Pos, 5> wr_positions_y = {{{0, false}, {-1, true}, {0, true}, {1, true}, {Size::board::y - 1, false}}};

  std::vector<PosOffset> wr_positions;
  for (Pos const& pox : wr_positions_x)
    for (Pos const& poy : wr_positions_y)
      wr_positions.emplace_back(pox, poy);

  // Generate positions that test every part of black move generation and
  // check that generate_neighbors generates precisely all the legal positions.
  // The generated child positions are stored in white_to_move_children.
  std::vector<Board> white_to_move_children;
  for (BlackKingSquare const& bk : bk_positions)
    for (PosOffset const& wkpo : wk_offsets)
    {
      int wkx = wkpo.x + bk.x_coord();
      int wky = wkpo.y + bk.y_coord();
      if (wkx < 0 || wkx >= Size::board::x || wky < 0 || wky >= Size::board::y)
        continue;
      WhiteKingSquare wk(wkx, wky);
      // Keep track of absolute rook squares already emitted for this bk/wk pair.
      std::set<std::pair<int, int>> seen_wr;
      for (PosOffset const& wrpo : wr_positions)
      {
        int wrx = wrpo.x + bk.x_coord();
        int wry = wrpo.y + bk.y_coord();
        if (wrx < 0 || wrx >= Size::board::x || wry < 0 || wry >= Size::board::y)
          continue;
        // Never try to generate moves after the white rook is already captured.
        if (wrx == bk.x_coord() && wry == bk.y_coord())
          continue;
        // Skip duplicate absolute rook square.
        auto const key = std::make_pair(wrx, wry);
        if (!seen_wr.insert(key).second)
          continue;
        WhiteRookSquare wr(wrx, wry);
        Board board(bk, wk, wr);
        constexpr Color to_move = black;
        std::cout << "Original position:\n";
        board.utf8art(std::cout, to_move, false);
        // Get the current x,y coordinates of all pieces.
        auto [cbk, cwk, cwr] = board.abbreviations();
        Graph::info_nodes_type& white_to_move = graph2.white_to_move();
        std::set<InfoIndex> legal_moves;
        int n = 0;
        {
          // Brute force all legal black king moves.
          using namespace coordinates;
          for (int bkx = cbk[x] - 1; bkx <= cbk[x] + 1; ++bkx)
            for (int bky = cbk[y] - 1; bky <= cbk[y] + 1; ++bky)
            {
              if (bkx == cbk[x] && bky == cbk[y])
                continue;
              if (bkx < 0 || bkx >= Size::board::x || bky < 0 || bky >= Size::board::y)
                continue;
              Board b({bkx, bky}, {cwk[x], cwk[y]}, {cwr[x], cwr[y]});
              if (white_to_move[b.as_index()].classification().is_legal())
              {
                legal_moves.insert(b.as_index());
                ++n;
              }
            }
        }
        ASSERT(legal_moves.size() == n);
        std::cout << "which has " << n << " legal moves.\n";
        Board::neighbors_type white_to_move_neighbors;
        int number_of_white_to_move_neighbors = board.generate_neighbors<Board::children, to_move>(white_to_move_neighbors);
        if (legal_moves.size() != number_of_white_to_move_neighbors)
        {
          std::cout << "Generated moves:" << std::endl;
          for (int i = 0; i < number_of_white_to_move_neighbors; ++i)
            white_to_move_neighbors[i].utf8art(std::cout, to_move.opponent());
        }
        // All possibly legal moves must be generated.
        ASSERT(legal_moves.size() == number_of_white_to_move_neighbors);
        for (int i = 0; i < number_of_white_to_move_neighbors; ++i)
        {
          // Verify that every generated move corresponds with one of the possible legal moves.
          ASSERT(legal_moves.find(white_to_move_neighbors[i].as_index()) != legal_moves.end());
          white_to_move_children.push_back(white_to_move_neighbors[i]);
        }
      }
    }

  WhiteKingSquare const wk_positions[] = {
    {0, 0},                                     // Corner
    {0, Size::board::y - 1},                    // Corner
    {Size::board::x - 1, 0},                    // Corner
    {Size::board::x - 1, Size::board::y - 1},   // Corner
    {0, center_y},                              // Edge (west)
    {Size::board::x - 1, center_y},             // Edge (east)
    {center_x, 0},                              // Edge (south)
    {center_x, Size::board::y - 1},             // Edge (north)
    {center_x, center_y}                        // Center (no edges)
  };

  std::vector<PosOffset> bk_offsets;
  bk_offsets.emplace_back(Pos{2 * center_x, false}, Pos{2 * center_y, false});   // Far away (no influence).
  // Two squares away.
  for (int i = -2; i <= 2; ++i)
  {
    bk_offsets.emplace_back(Pos{i, true}, Pos{-2, true});
    bk_offsets.emplace_back(Pos{i, true}, Pos{2, true});
    if (i > -2 && i < 2)
    {
      bk_offsets.emplace_back(Pos{-2, true}, Pos{i, true});
      bk_offsets.emplace_back(Pos{2, true}, Pos{i, true});
    }
  }

  // Generate positions that test every part of white move generation and
  // check that generate_neighbors generates precisely all the legal positions.
  std::vector<Board> black_to_move_children;
  for (WhiteKingSquare const& wk : wk_positions)
    for (PosOffset const& bkpo : bk_offsets)
    {
      int bkx = bkpo.x + wk.x_coord();
      int bky = bkpo.y + wk.y_coord();
      if (bkx < 0 || bkx >= Size::board::x || bky < 0 || bky >= Size::board::y)
        continue;
      BlackKingSquare bk(bkx, bky);
      // Keep track of absolute rook squares already emitted for this bk/wk pair.
      std::set<std::pair<int, int>> seen_wr;
      for (PosOffset const& wrpo : wr_positions)
      {
        int wrx = wrpo.x + bk.x_coord();
        int wry = wrpo.y + bk.y_coord();
        if (wrx < 0 || wrx >= Size::board::x || wry < 0 || wry >= Size::board::y)
          continue;
        if (wrx == wk.x_coord() && wry == wk.y_coord())
          continue;
        // Never try to generate moves after the white rook is already captured.
        if (wrx == bk.x_coord() && wry == bk.y_coord())
          continue;
        // Skip duplicate absolute rook square.
        auto const key = std::make_pair(wrx, wry);
        if (!seen_wr.insert(key).second)
          continue;
        WhiteRookSquare wr(wrx, wry);
        Board board(bk, wk, wr);
        constexpr Color to_move = white;
        // Skip all illegal positions.
        Graph::info_nodes_type& white_to_move = graph2.white_to_move();
        if (!white_to_move[board.as_index()].classification().is_legal())
          continue;
        std::cout << "Original position:\n";
        board.utf8art(std::cout, to_move, false);
        // Get the current x,y coordinates of all pieces.
        auto [cbk, cwk, cwr] = board.abbreviations();
        Graph::info_nodes_type& black_to_move = graph2.black_to_move();
        std::set<InfoIndex> legal_moves;
        int n = 0;
        {
          // Brute force all legal white king moves.
          using namespace coordinates;
          for (int wkx = cwk[x] - 1; wkx <= cwk[x] + 1; ++wkx)
            for (int wky = cwk[y] - 1; wky <= cwk[y] + 1; ++wky)
            {
              if (wkx == cwk[x] && wky == cwk[y])
                continue;
              if (wkx < 0 || wkx >= Size::board::x || wky < 0 || wky >= Size::board::y)
                continue;
              Board b({cbk[x], cbk[y]}, {wkx, wky}, {cwr[x], cwr[y]});
              if (black_to_move[b.as_index()].classification().is_legal())
              {
                legal_moves.insert(b.as_index());
                ++n;
              }
            }
          // Brute force all legal white rook moves.
          int wry = cwr[y];
          for (int wrx = cwr[x] + 1; wrx < Size::board::x; ++wrx)
          {
            // Don't move through the white king.
            if (wrx == cwk[x] && wry == cwk[y])
              break;
            Board b({cbk[x], cbk[y]}, {cwk[x], cwk[y]}, {wrx, wry});
            if (black_to_move[b.as_index()].classification().is_legal())
            {
              legal_moves.insert(b.as_index());
              ++n;
            }
          }
          for (int wrx = cwr[x] - 1; wrx >= 0; --wrx)
          {
            // Don't move through the white king.
            if (wrx == cwk[x] && wry == cwk[y])
              break;
            Board b({cbk[x], cbk[y]}, {cwk[x], cwk[y]}, {wrx, wry});
            if (black_to_move[b.as_index()].classification().is_legal())
            {
              legal_moves.insert(b.as_index());
              ++n;
            }
          }
          int wrx = cwr[x];
          for (int wry = cwr[y] + 1; wry < Size::board::y; ++wry)
          {
            // Don't move through the white king.
            if (wrx == cwk[x] && wry == cwk[y])
              break;
            Board b({cbk[x], cbk[y]}, {cwk[x], cwk[y]}, {wrx, wry});
            if (black_to_move[b.as_index()].classification().is_legal())
            {
              legal_moves.insert(b.as_index());
              ++n;
            }
          }
          for (int wry = cwr[y] - 1; wry >= 0; --wry)
          {
            // Don't move through the white king.
            if (wrx == cwk[x] && wry == cwk[y])
              break;
            Board b({cbk[x], cbk[y]}, {cwk[x], cwk[y]}, {wrx, wry});
            if (black_to_move[b.as_index()].classification().is_legal())
            {
              legal_moves.insert(b.as_index());
              ++n;
            }
          }
        }
        ASSERT(legal_moves.size() == n);
        std::cout << "which has " << n << " legal moves.\n";
        Board::neighbors_type black_to_move_neighbors;
        int number_of_black_to_move_neighbors = board.generate_neighbors<Board::children, to_move>(black_to_move_neighbors);
        if (legal_moves.size() != number_of_black_to_move_neighbors)
        {
          std::cout << "Generated moves:" << std::endl;
          for (int i = 0; i < number_of_black_to_move_neighbors; ++i)
            black_to_move_neighbors[i].utf8art(std::cout, to_move.opponent());
        }
        // All possibly legal moves must be generated.
        ASSERT(legal_moves.size() == number_of_black_to_move_neighbors);
        for (int i = 0; i < number_of_black_to_move_neighbors; ++i)
        {
          // Verify that every generated move corresponds with one of the possible legal moves.
          ASSERT(legal_moves.find(black_to_move_neighbors[i].as_index()) != legal_moves.end());
          black_to_move_children.push_back(black_to_move_neighbors[i]);
        }
      }
    }

    // Run over all generated positions to test every part of black-to-move parent generation
    // and check that generate_neighbors generates precisely all the legal positions.
    std::cout << "Running over " << white_to_move_children.size() << " (child) positions." << std::endl;
    for (int i = 0; i < (int)white_to_move_children.size(); ++i)
    {
      Board board = white_to_move_children[i];
      constexpr Color to_move = white;
      // Get the current x,y coordinates of all pieces.
      auto [cbk, cwk, cwr] = board.abbreviations();
      if (cbk == cwr)
        continue;       // Can't find parent positions where the white rook was already taken.

      std::cout << "Original child position:\n";
      board.utf8art(std::cout, to_move, false);

      Graph::info_nodes_type& black_to_move = graph2.black_to_move();
      std::set<InfoIndex> legal_moves;
      int n = 0;
      {
        // Brute force all legal black king moves.
        using namespace coordinates;
        for (int bkx = cbk[x] - 1; bkx <= cbk[x] + 1; ++bkx)
          for (int bky = cbk[y] - 1; bky <= cbk[y] + 1; ++bky)
          {
            if (bkx == cbk[x] && bky == cbk[y])
              continue;
            if (bkx < 0 || bkx >= Size::board::x || bky < 0 || bky >= Size::board::y)
              continue;
            Board b({bkx, bky}, {cwk[x], cwk[y]}, {cwr[x], cwr[y]});
            if (black_to_move[b.as_index()].classification().is_legal())
            {
              legal_moves.insert(b.as_index());
              ++n;
            }
          }
      }
      ASSERT(legal_moves.size() == n);
      std::cout << "which has " << n << " legal moves.\n";
      Board::neighbors_type black_to_move_parents;
      int number_of_parents = board.generate_neighbors<Board::parents, to_move.opponent()>(black_to_move_parents);
      if (legal_moves.size() != number_of_parents)
      {
        std::cout << "Generated parents:" << std::endl;
        for (int i = 0; i < number_of_parents; ++i)
          black_to_move_parents[i].utf8art(std::cout, to_move.opponent());
      }
      // All possibly legal moves must be generated.
      ASSERT(legal_moves.size() == number_of_parents);
      for (int i = 0; i < number_of_parents; ++i)
      {
        // Verify that every generated move corresponds with one of the possible legal moves.
        ASSERT(legal_moves.find(black_to_move_parents[i].as_index()) != legal_moves.end());
      }
    }

    // Run over all generated positions to test every part of white-to-move parent generation
    // and check that generate_neighbors generates precisely all the legal positions.
    std::cout << "Running over " << black_to_move_children.size() << " (child) positions." << std::endl;
    for (int i = 0; i < (int)black_to_move_children.size(); ++i)
    {
      Board board = black_to_move_children[i];
      constexpr Color to_move = black;
      // Get the current x,y coordinates of all pieces.
      auto [cbk, cwk, cwr] = board.abbreviations();
      if (cbk == cwr)
        continue;       // Can't find parent positions where the white rook was already taken.

      std::cout << "Original child position:\n";
      board.utf8art(std::cout, to_move, false);

      Graph::info_nodes_type& white_to_move = graph2.white_to_move();
      std::set<InfoIndex> legal_moves;
      int n = 0;
      {
        // Brute force all legal white king moves.
        using namespace coordinates;
        for (int wkx = cwk[x] - 1; wkx <= cwk[x] + 1; ++wkx)
          for (int wky = cwk[y] - 1; wky <= cwk[y] + 1; ++wky)
          {
            if (wkx == cwk[x] && wky == cwk[y])
              continue;
            if (wkx < 0 || wkx >= Size::board::x || wky < 0 || wky >= Size::board::y)
              continue;
            Board b({cbk[x], cbk[y]}, {wkx, wky}, {cwr[x], cwr[y]});
            if (white_to_move[b.as_index()].classification().is_legal())
            {
              legal_moves.insert(b.as_index());
              ++n;
            }
          }
        // Brute force all legal white rook moves.
        int wry = cwr[y];
        for (int wrx = cwr[x] + 1; wrx < Size::board::x; ++wrx)
        {
          // Don't move through the white king.
          if (wrx == cwk[x] && wry == cwk[y])
            break;
          // Don't add parent positions where the rook is taken.
          if (wrx == cbk[x] && wry == cbk[y])
            break;
          Board b({cbk[x], cbk[y]}, {cwk[x], cwk[y]}, {wrx, wry});
          if (white_to_move[b.as_index()].classification().is_legal())
          {
            legal_moves.insert(b.as_index());
            ++n;
          }
        }
        for (int wrx = cwr[x] - 1; wrx >= 0; --wrx)
        {
          // Don't move through the white king.
          if (wrx == cwk[x] && wry == cwk[y])
            break;
          // Don't add parent positions where the rook is taken.
          if (wrx == cbk[x] && wry == cbk[y])
            break;
          Board b({cbk[x], cbk[y]}, {cwk[x], cwk[y]}, {wrx, wry});
          if (white_to_move[b.as_index()].classification().is_legal())
          {
            legal_moves.insert(b.as_index());
            ++n;
          }
        }
        int wrx = cwr[x];
        for (int wry = cwr[y] + 1; wry < Size::board::y; ++wry)
        {
          // Don't move through the white king.
          if (wrx == cwk[x] && wry == cwk[y])
            break;
          // Don't add parent positions where the rook is taken.
          if (wrx == cbk[x] && wry == cbk[y])
            break;
          Board b({cbk[x], cbk[y]}, {cwk[x], cwk[y]}, {wrx, wry});
          if (white_to_move[b.as_index()].classification().is_legal())
          {
            legal_moves.insert(b.as_index());
            ++n;
          }
        }
        for (int wry = cwr[y] - 1; wry >= 0; --wry)
        {
          // Don't move through the white king.
          if (wrx == cwk[x] && wry == cwk[y])
            break;
          // Don't add parent positions where the rook is taken.
          if (wrx == cbk[x] && wry == cbk[y])
            break;
          Board b({cbk[x], cbk[y]}, {cwk[x], cwk[y]}, {wrx, wry});
          if (white_to_move[b.as_index()].classification().is_legal())
          {
            legal_moves.insert(b.as_index());
            ++n;
          }
        }
      }
      ASSERT(legal_moves.size() == n);
      std::cout << "which has " << n << " legal moves.\n";
      Board::neighbors_type white_to_move_parents;
      int number_of_parents = board.generate_neighbors<Board::parents, to_move.opponent()>(white_to_move_parents);
      if (legal_moves.size() != number_of_parents)
      {
        std::cout << "Generated parents:" << std::endl;
        for (int i = 0; i < number_of_parents; ++i)
          white_to_move_parents[i].utf8art(std::cout, to_move.opponent());
      }
      // All possibly legal moves must be generated.
      ASSERT(legal_moves.size() == number_of_parents);
      for (int i = 0; i < number_of_parents; ++i)
      {
        // Verify that every generated move corresponds with one of the possible legal moves.
        ASSERT(legal_moves.find(white_to_move_parents[i].as_index()) != legal_moves.end());
      }
    }
#endif

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
