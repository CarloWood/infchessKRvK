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
#if 0
  // All positions where black is mate (i.e. black is to move).
  std::vector<Board> mate_positions;

  // Generate all positions that are already mate.

  // Generate all positions with the black king in the corner.
  for (int wrc = 2; wrc < board_size; ++wrc)
  {
    mate_positions.emplace_back(board_size, Square{0, 0}, Square{2, 0}, Square{0, wrc});
    mate_positions.emplace_back(board_size, Square{0, 0}, Square{2, 1}, Square{0, wrc});
    mate_positions.emplace_back(board_size, Square{0, 0}, Square{0, 2}, Square{wrc, 0});
    mate_positions.emplace_back(board_size, Square{0, 0}, Square{1, 2}, Square{wrc, 0});
  }

  // Generate all positions with the black king at the edge.
  for (int bkx = 1; bkx < board_size; ++bkx)
  {
    for (int wrx = 0; wrx < board_size; ++wrx)
      if (std::abs(wrx  - bkx) > 1)
      {
        mate_positions.emplace_back(board_size, Square{bkx, 0}, Square{bkx, 2}, Square{wrx, 0});
        mate_positions.emplace_back(board_size, Square{0, bkx}, Square{2, bkx}, Square{0, wrx});
      }
  }

  // Store all positions that are already mate in our map and the mate_in_ply_[0] vector.
  mate_in_ply_.resize(1);
  for (Board const& board : mate_positions)
  {
    Dout(dc::notice, "Trying to find " << board << " in black_to_move_.");
    auto iter = black_to_move_.find(board);
    ASSERT(iter != black_to_move_.end());
    ASSERT(iter->second.is_mate());
    mate_in_ply_[0].push_back(iter);
  }
#endif
}

#if 0
std::vector<Board> Graph::adjacent_positions(Board const& current_board, Color current_to_move)
{
  DoutEntering(dc::notice, "Graph::adjacent_positions(" << current_board << ", " << current_to_move << ")");

  std::vector<Board> result;

  // Set `to_move` to the player that is to move in the positions that we are generating.
  Color to_move = current_to_move.opponent();
  Square const cbk = current_board.black_king();
  Square const cwk = current_board.white_king();    // The 'c' stands for Current, Constant or Canonical - your pick.
  Square const cwr = current_board.white_rook();
  // The latest positions that are already known to be mate, are mate in k ply.
  int const k = mate_in_ply_.size() - 2;
#ifdef CWDEBUG
  nodes_type const& nodes = current_to_move == black ? black_to_move_ : white_to_move_;
  auto iter = nodes.find(current_board);
  // Only call adjacent_positions for boards that are from one of the (correct) map(s).
  ASSERT(iter != nodes.end());
  // mate_in_ply_[k + 1] is being generated, so current_board should be mate in k ply.
  ASSERT(k == iter->second.ply());
#endif

  if (to_move == black)
  {
    // Run over all possible (legal) adjacent positions that the black king could have moved from.
    int index = 0;
    for (Square bk : KingMoves{current_board, black})
    {
      // Construct the preceding board, where black is to play.
      Board adjacent_board(board_size_, bk, cwk, cwr);
      Dout(dc::adjacent, "Adjacent board (created from index " << index << ") - derived from " << current_board << ":");
      Debug(adjacent_board.debug_utf8art(dc::adjacent));
      bool valid = true;
      // All adjacent positions should have higher (or equal) ply.
#ifdef CWDEBUG
      auto iter = black_to_move_.find(adjacent_board);
      // In any position that is mate in k+1 ply the black king has more than one possible move
      // (which is most positions) we will find that position back tracking from each of those
      // possibilities. Therefore it is possible we found this position multiple times already.
      //
      // For example, the follow positions are all mate in 3 ply (white to play):
      //
      //   ┏━0━1━2━3━
      // 0 ┃ ·   ♔      1. K(1,2) K(1,0) (only move)
      // 1 ┃   ·   ·    2. R(3,0)#
      // 2 ┃ ·   ♚ ♜
      //
      //   ┏━0━1━2━3━
      // 0 ┃ ♔   ·      1. K(1,2) K(1,0) (only move)
      // 1 ┃   ·   ·    2. R(3,0)#
      // 2 ┃ ·   ♚ ♜
      //
      //   ┏━0━1━2━3━
      // 0 ┃ ·   ·      1. K(2,1) K(0,0) (only move)
      // 1 ┃ ♔ ·   ·    2. R(0,2)#
      // 2 ┃ ·   ♚ ♜
      //
      // Therefore we find
      //
      //   ┏━0━1━2━3━
      // 0 ┃ · ♔ ·      1. ... K(2,0) and mate in 3 ply
      // 1 ┃   ·   ·    1. ... K(0,0) and mate in 3 ply
      // 2 ┃ ·   ♚ ♜    1. ... K(0,1) and mate in 3 ply
      //
      // three times, as being an adjacent position, once for each of the three first positions given.
      ASSERT(iter == black_to_move_.end() || iter->second.ply() == k + 1);
#endif
      Dout(dc::alternate, " .---Potential alternate positions:");
#ifdef CWDEBUG
      bool debug_off = !DEBUGCHANNELS::dc::alternate.is_on();
      if (debug_off)
        libcwd::libcw_do.off();
#endif
      {
#ifdef CWDEBUG
        NAMESPACE_DEBUG::Mark __mark;
#endif
        Square const cwk2 = adjacent_board.white_king();
        Square const cwr2 = adjacent_board.white_rook();
        int index2 = 0;
        // Generate normal (forward) moves.
        for (Square bk : KingMoves{adjacent_board, black, true})
        {
          using namespace coordinates;

          // Generate an alternative position that black could have chosen from the position of adjacent_board.
          // On this board it is white to move.
          Board alternate_board(board_size_, bk, cwk2, cwr2);

          // Reject `adjacent_board` if black could have avoided it by doing a different move, by stepping over one of the virtual edges.
          if (bk[x] == board_size_ || bk[y] == board_size_)
          {
            Dout(dc::alternate|dc::adjacent, "Black can escape outside the board with K" << bk <<
                "! Rejecting position " << adjacent_board << ".");
            valid = false;
            break;
          }
          Dout(dc::alternate, "alternate position (created from index " << index2 << ") - derived from " << adjacent_board << ":");
          Debug(alternate_board.debug_utf8art(dc::alternate));
          // If this alternate move is unknown yet, then it will be a position in which it takes longer to mate black.
          if (white_to_move_.find(alternate_board) == white_to_move_.end())
          {
            Dout(dc::alternate|dc::adjacent, "There is a better move for black: K" << bk << "! Rejecting position " << adjacent_board << ".");
            valid = false;
            break;
          }
          ++index2;
        }
      }
      if (debug_off)
        libcwd::libcw_do.on();
      Dout(dc::alternate, " `---End of potential alternate positions.");
      if (valid)
      {
        result.push_back(adjacent_board);
        Dout(dc::adjacent, "Added adjacent board (created from index " << index << ") - derived from " <<
            current_board << " to index " << (result.size() - 1));
        ++index;
      }
    }
  }
  else
  {
    // Run over all possible (legal) adjacent positions that the white king could have moved from.
    {
      int index = 0;
      for (Square wk : KingMoves{current_board, white})
      {
        result.emplace_back(board_size_, cbk, wk, cwr);
        Dout(dc::adjacent, "Added adjacent board (created from index " << index << ") - derived from " << current_board << " :");
        Debug(result.back().debug_utf8art(dc::adjacent));
        ++index;
      }
    }
    // Run over all possible (legal) adjacent positions that the white rook could have moved from.
    int index = 0;
    for (Square wr : RookMoves{current_board})
    {
      if (wr == cbk)    // Don't consider the drawn position where the rook was already captured.
        continue;
      result.emplace_back(board_size_, cbk, cwk, wr);
      Dout(dc::adjacent, "Added adjacent board (created from index " << index << ") - derived from " << current_board << " :");
      Debug(result.back().debug_utf8art(dc::adjacent));
      ++index;
    }
  }

  return result;
}
#endif

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
  generate_edges_with_color<black>();
  generate_edges_with_color<white>();
}

#if 0
void Graph::generate(int ply)
{
  DoutEntering(dc::notice, "Graph::generate(" << ply << ")");
  ASSERT(ply > 0);

  // Generate the positions that are mate in ply - 1 first.
  if (mate_in_ply_.size() < ply)
    generate(ply - 1);

  // ply should always be the last one.
  ASSERT(ply == mate_in_ply_.size());
  int const number_of_mate_in_ply_minus_one_positions = mate_in_ply_[ply - 1].size();

  Dout(dc::notice, "There are " << number_of_mate_in_ply_minus_one_positions << " positions that are mate in " << (ply - 1) << " ply.");

  // Store all positions that are mate in `ply` moves in a new positions_type.
  positions_type& new_positions = mate_in_ply_.emplace_back();

  // Run over all positions that are mate in ply minus one.
  for (nodes_type::const_iterator const& iter : mate_in_ply_[ply - 1])
  {
    Board const& current_position(iter->first);
    // We can deduce from the ply whose move it is in the current position.
    Color current_to_move = ply % 2 == 1 ? black : white;

    Dout(dc::notice, "Mate in " << (ply - 1) << " position (" << current_position << ", " << current_to_move << " to move):");
    Debug(current_position.debug_utf8art(dc::notice));

    std::vector<Board> adjacent_boards = adjacent_positions(current_position, current_to_move);
    Dout(dc::notice, "Returned: " << adjacent_boards.size() << " positions.");

    // In adjacent_position the opposite color is to move, which will be added to `nodes`.
    nodes_type& current_nodes = current_to_move == black ? black_to_move_ : white_to_move_;
    nodes_type& nodes = current_to_move == white ? black_to_move_ : white_to_move_;

    for (Board const& adjacent_position : adjacent_boards)
    {
      auto ibp = nodes.try_emplace(adjacent_position, ply);
      bool need_edge = false;
      if (ibp.second)
      {
        // If `adjacent_position` doesn't already exist in nodes then
        // apparently one can't reach mate faster than in `ply` moves.
        // Hence it must be mate in `ply` exactly since we can reach
        // current_position from it, which is mate in `ply - 1` moves.
        mate_in_ply_[ply].push_back(ibp.first);
        need_edge = true;
      }
      else
      {
        // If `adjacent_position` already exists in the map, then it is possible
        // that it was added because from `adjacent_position` we can reach a position,
        // in one move, that is mate in `ply - 1` moves where that other position is not
        // the current position.
        //
        // For example, assume B is the current position, ply = 3 and `adjacent_position` is D:
        //
        //          Q         (mate in one ply (white to move)).
        //          |
        //      A   B         (mate in two ply (black to move)).
        //     / \ / \
        //    C   D   E       (mate in three ply (white to move)).
        //
        // While A was the current position, we found adjacent positions C and D
        // and added C and D to white_to_move_ as well as a link from C to A and from D to A.
        //
        // Now B is the current position, we find that D already exists in white_to_move_
        // but still need to add the link from D to B.
        //
        // Note that we need to check that D is indeed a position that is mate in `ply` moves,
        // because from B we will also find adjacent positions that are mate in `ply - 2` moves (Q).
        need_edge = ibp.first->second.ply() == ply;
      }
      if (need_edge)
      {
        auto current_position_iter = current_nodes.find(current_position);
        Dout(dc::edge, "Adding an edge from " << ibp.first->first << " to " << current_position << ".");
        // Add a link from the adjacent position to the current position.
        ibp.first->second.add_edge(current_position_iter);
      }
    }
  }
}
#endif

} // namespace version0
