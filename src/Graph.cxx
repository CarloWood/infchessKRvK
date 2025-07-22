#include "sys.h"
#include "Graph.h"
#include "KingMoves.h"
#include "RookMoves.h"
#include "debug.h"

NAMESPACE_DEBUG_CHANNELS_START
channel_ct alternate("ALTERNATE");
channel_ct adjacent("ADJACENT");
channel_ct edge("EDGE");
NAMESPACE_DEBUG_CHANNELS_END

Graph::Graph()
{
  std::vector<Board> mate_positions;

  // Generate all positions that are already mate.
  for (int wrc = 2; wrc < Board::horizontal_limit; ++wrc)
  {
    for (int mirrored = 0; mirrored <= 1; ++mirrored)
    {
      mate_positions.emplace_back(Square{0, 0}, Square{2, 0}, Square{0, wrc}, black, mirrored);
      mate_positions.emplace_back(Square{0, 0}, Square{2, 1}, Square{0, wrc}, black, mirrored);
    }
  }
  for (int kn = 1; kn < Board::horizontal_limit; ++kn)
  {
    for (int wrn = 0; wrn < Board::horizontal_limit; ++wrn)
      if (std::abs(wrn  - kn) > 1)
      {
        for (int mirrored = 0; mirrored <= 1; ++mirrored)
          mate_positions.emplace_back(Square{kn, 0}, Square{kn, 2}, Square{wrn, 0}, black, mirrored);
      }
  }

  // Store all positions that are already mate in our map and the mate_in_ply_[0] vector.
  mate_in_ply_.resize(1);
  for (Board const& board : mate_positions)
  {
    auto ibp = nodes_.try_emplace(board, 0);
    // We expect that none of the added Board's are flipped because we add the non-flipped one first.
    ASSERT(!ibp.first->first.print_flipped());
    // Only store positions that were newly inserted in the vector.
    if (ibp.second)
      mate_in_ply_[0].push_back(ibp.first);
  }
}

std::vector<Board> Graph::adjacent_positions(Board const& current_board)
{
  DoutEntering(dc::notice, "Graph::adjacent_positions(" << current_board << ")");

  std::vector<Board> result;

  Color to_play = current_board.to_play().opponent();
  Square const cwk = current_board.wK().pos();    // The 'c' stands for Current, Constant or Canonical - your pick.
  Square const cwr = current_board.wR().pos();
  Square const cbk = current_board.bK().pos();
  bool const cflipped = current_board.print_flipped();
  // The latest positions that are already known to be mate, are mate in k ply.
  int const k = mate_in_ply_.size() - 2;
#ifdef CWDEBUG
  auto iter = nodes_.find(current_board);
  // Only call adjacent_positions for boards that are from nodes_.
  ASSERT(iter != nodes_.end());
  // mate_in_ply_[k + 1] is being generated, so current_board should be mate in k ply.
  ASSERT(k == iter->second.ply());
#endif

  if (to_play == black)
  {
    // Run over all possible (legal) adjacent positions that the black king could have moved from.
    int index = 0;
    for (Square bk : KingMoves{current_board, black})
    {
      Board adjacent_board(bk, cwk, cwr, black, cflipped);
      Dout(dc::adjacent, "Adjacent board (created from index " << index << ") - derived from " << current_board << ":");
      Debug(adjacent_board.debug_utf8art(dc::adjacent));
      bool valid = true;
      // All adjacent positions should have higher (or equal) ply.
#ifdef CWDEBUG
      auto iter = nodes_.find(adjacent_board);
      // It is possible we already added the flipped version, which will be the same ply of course.
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
      // three times, as being a adjacent position, once for each of the three first positions given.
      ASSERT(iter == nodes_.end() || iter->second.ply() == k + 1);
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
        Square const cwk2 = adjacent_board.wK().pos();
        Square const cwr2 = adjacent_board.wR().pos();
        int index2 = 0;
        // Generate normal (forward) moves.
        for (Square bk : KingMoves{adjacent_board, black, true})
        {
          Board alternate_board(bk, cwk2, cwr2, white, adjacent_board.print_flipped());
          if (bk.n == Board::horizontal_limit || bk.m == Board::vertical_limit)
          {
            Dout(dc::alternate|dc::adjacent, "Black can escape outside the board with K" << bk <<
                "! Rejecting position " << adjacent_board << ".");
            valid = false;
            break;
          }
          Dout(dc::alternate, "alternate position (created from index " << index2 << ") - derived from " << adjacent_board << ":");
          Debug(alternate_board.debug_utf8art(dc::alternate));
          // If this alternate move is unknown yet, then it will be a position in which it takes longer to mate black.
          if (nodes_.find(alternate_board) == nodes_.end())
          {
            Dout(dc::alternate|dc::adjacent, "There is a better move for black K" << bk << "! Rejecting position " << adjacent_board << ".");
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
        result.emplace_back(cbk, wk, cwr, white, cflipped);
        Dout(dc::adjacent, "Added adjacent board (created from index " << index << ") - derived from " << current_board << " :");
        Debug(result.back().debug_utf8art(dc::adjacent));
        ++index;
      }
    }
    // Run over all possible (legal) adjacent positions that the white rook could have moved from.
    int index = 0;
    for (Square wr : RookMoves{current_board})
    {
      result.emplace_back(cbk, cwk, wr, white, cflipped);
      Dout(dc::adjacent, "Added adjacent board (created from index " << index << ") - derived from " << current_board << " :");
      Debug(result.back().debug_utf8art(dc::adjacent));
      ++index;
    }
  }

  return result;
}

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

  // Store all positions that are mate in `ply` ply in a new positions_type.
  positions_type& new_positions = mate_in_ply_.emplace_back();

  // Run over all positions that are mate in ply minus one.
  for (nodes_type::const_iterator const& iter : mate_in_ply_[ply - 1])
  {
    Board const& current_position(iter->first);
    Dout(dc::notice, "Mate in " << (ply - 1) << " position (" << current_position << "):");
    Debug(current_position.debug_utf8art(dc::notice));

    std::vector<Board> adjacent_boards = adjacent_positions(current_position);
    Dout(dc::notice, "Returned: " << adjacent_boards.size() << " positions.");

    for (Board const& adjacent_position : adjacent_boards)
    {
      auto ibp = nodes_.try_emplace(adjacent_position, ply);
      bool need_edge = false;
      if (ibp.second)
      {
        // If `adjacent_position` doesn't already exist in nodes_ then
        // apparently one can't reach mate faster than in `ply` ply.
        // Hence it must be mate in `ply` exactly since we can reach
        // current_position from it, which is mate in `ply - 1` ply.
        mate_in_ply_[ply].push_back(ibp.first);
        need_edge = true;
      }
      else
      {
        // If `adjacent_position` already exists in the map, then it is possible
        // that it was added because from `adjacent_position` we can reach a position,
        // in one move, that is mate in `ply - 1` ply where that other position is not
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
        // and added C and D to nodes_ as well as a link from C to A and from D to A.
        //
        // Now B is the current position, we find that D already exists in nodes_
        // but still need to add the link from D to B.
        //
        // Note that we to check that D is indeed a position that is mate in `ply` ply,
        // because from B we will also find adjacent positions that are mate in `ply - 2` ply (Q).
        need_edge = ibp.first->second.ply() == ply;
      }
      if (need_edge)
      {
        auto current_position_iter = nodes_.find(current_position);
        Dout(dc::edge, "Adding an edge from " << ibp.first->first << " to " << current_position << ".");
        ibp.first->second.add_edge(current_position_iter);
      }
    }
  }
}
