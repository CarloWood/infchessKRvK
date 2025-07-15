#include "sys.h"
#include "Graph.h"
#include "KingMoves.h"
#include "RookMoves.h"
#include "debug.h"

NAMESPACE_DEBUG_CHANNELS_START
channel_ct alternate("ALTERNATE");
channel_ct adjacent("ADJACENT");
channel_ct preceding("PRECEDING");
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

std::vector<Board> Graph::preceding_positions(Board const& current_board)
{
  DoutEntering(dc::notice, "Graph::preceding_positions(" << current_board << ")");

  std::vector<Board> result;

  Color to_play = current_board.to_play().next();
  Square const cwk = current_board.wK().pos();    // The 'c' stands for Current, Constant or Canonical - your pick.
  Square const cwr = current_board.wR().pos();
  Square const cbk = current_board.bK().pos();
  bool const cflipped = current_board.print_flipped();
  // The latest positions that are already known to be mate, are mate in k ply.
  int const k = mate_in_ply_.size() - 2;
#ifdef CWDEBUG
  auto iter = nodes_.find(current_board);
  // Only call preceding_positions for boards that are from nodes_.
  ASSERT(iter != nodes_.end());
  // mate_in_ply_[k + 1] is being generated, so current_board should be mate in k ply.
  ASSERT(k == iter->second.ply_);
#endif

  if (to_play == black)
  {
    // Run over all possible (legal) preceding positions that the black king could have moved from.
    int index = 0;
    for (Square bk : KingMoves{current_board, black})
    {
      Board adjacent_board(bk, cwk, cwr, black, cflipped);
      Dout(dc::adjacent, "Adjacent board (created from index " << index << ") - derived from " << current_board << ":");
      Debug(adjacent_board.debug_utf8art(dc::adjacent));
      // black king:(0, 0), white king:(3, 1), rook:(5, 1), black to play
      if (bk == Square{0, 0} && cwk == Square{3, 1} && cwr == Square{5, 1} && to_play == black)
        Dout(dc::adjacent, "TEST CASE!");
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
      // three times, as being a preceding position, once for each of the three first positions given.
      ASSERT(iter == nodes_.end() || iter->second.ply_ == k + 1);
#endif
      Dout(dc::alternate, " .---Potential alternate positions:");
      {
#ifdef CWDEBUG
        NAMESPACE_DEBUG::Mark __mark;
#endif
        Square const cwk2 = adjacent_board.wK().pos();
        Square const cwr2 = adjacent_board.wR().pos();
        int index2 = 0;
        for (Square bk : KingMoves{adjacent_board, black})
        {
          Board alternate_board(bk, cwk2, cwr2, white, adjacent_board.print_flipped());
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
      Dout(dc::alternate, " `---End of potential alternate positions.");
      if (valid)
      {
        result.push_back(adjacent_board);
        Dout(dc::adjacent, "Added adjacent board (created from index " << index << ") - derived from " << current_board << " :");
        Debug(result.back().debug_utf8art(dc::adjacent));
        ++index;
      }
    }
  }
  else
  {
    // Run over all possible (legal) preceding positions that the white king could have moved from.
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
    // Run over all possible (legal) preceding positions that the white rook could have moved from.
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

    std::vector<Board> preceding_boards = preceding_positions(current_position);
    Dout(dc::notice, "Returned: " << preceding_boards.size() << " positions.");

    {
#ifdef CWDEBUG
      Dout(dc::preceding, " .---Preceding positions:");
      NAMESPACE_DEBUG::Mark __mark;
#endif
      for (Board const& board : preceding_boards)
      {
        auto ibp = nodes_.try_emplace(board, ply);
        if (ibp.second)
          mate_in_ply_[ply].push_back(ibp.first);
        Dout(dc::preceding, "Predecing board:");
        Debug(ibp.first->first.debug_utf8art(dc::preceding));
      }
    }
    Dout(dc::preceding, " `---End of preceding positions.");
  }
}
