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
      mate_positions.emplace_back(Square{0, 0}, Square{0, 2}, Square{wrc, 0}, black, mirrored);
      mate_positions.emplace_back(Square{0, 0}, Square{1, 2}, Square{wrc, 0}, black, mirrored);
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

  // Store all positions that are already mate in our map.
  data_.resize(1);
  for (Board const& board : mate_positions)
  {
    auto ibp = data_[0].try_emplace(board, 0);
    // We expect that none of the added Board's are flipped because
    // we add the non-flipped one first.
    ASSERT(!ibp.first->first.print_flipped());
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
  // Get the latest positions that are already known to be mate in `ply` ply.
  int const ply = data_.size() - 2;
  // data_[ply + 1] is being generated.
  ASSERT(ply >= 0);
  map_type const& mate_in_ply_positions = data_[ply];
  ASSERT(!mate_in_ply_positions.empty());

  if (to_play == black)
  {
    // Run over all possible (legal) preceding positions that the black king could have moved from.
    int index = 0;
    for (Square bk : KingMoves{current_board, black})
    {
      Board adjacent_board(bk, cwk, cwr, black, cflipped);
      Dout(dc::adjacent, "Adjacent board (created from index " << index << "):");
      Debug(adjacent_board.debug_utf8art(dc::adjacent));
      bool valid = true;
      // All adjacent positions should have higher ply.
      ASSERT(mate_in_ply_positions.find(adjacent_board) == mate_in_ply_positions.end());
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
          Dout(dc::alternate, "alternate position (created from index " << index2 << "):");
          Debug(alternate_board.debug_utf8art(dc::alternate));
          if (mate_in_ply_positions.find(alternate_board) == mate_in_ply_positions.end())
          {
            Dout(dc::alternate|dc::adjacent, "There is a better move for black K" << bk << "! Rejecting position " << adjacent_board << ".");
#if 0
            Dout(dc::notice, "Could not find:");
            Debug(alternate_board.debug_utf8art(dc::notice));
            Dout(dc::notice, "in the map:");
            Dout(dc::notice, " .---Map contents for ply " << ply << ":");
            {
#ifdef CWDEBUG
              NAMESPACE_DEBUG::Mark __mark;
#endif
              for (map_type::value_type const& value : mate_in_ply_positions)
              {
                Debug(value.first.debug_utf8art(dc::notice));
              }
            }
            Dout(dc::notice, " `--End of map contents for ply " << ply << ".");
#endif
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
        Dout(dc::adjacent, "Added adjacent board (created from index " << index << "):");
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
        Dout(dc::adjacent, "Added adjacent board (created from index " << index << "):");
        Debug(result.back().debug_utf8art(dc::adjacent));
        ++index;
      }
    }
    // Run over all possible (legal) preceding positions that the white rook could have moved from.
    int index = 0;
    for (Square wr : RookMoves{current_board})
    {
      result.emplace_back(cbk, cwk, wr, white, cflipped);
      Dout(dc::adjacent, "Added adjacent board (created from index " << index << "):");
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
  if (data_.size() < ply)
    generate(ply - 1);

  // ply should always be the last one.
  ASSERT(ply == data_.size());
  int const number_of_mate_in_ply_minus_one_positions = data_[ply - 1].size();

  Dout(dc::notice, "There are " << number_of_mate_in_ply_minus_one_positions << " positions that are mate in " << (ply - 1) << " ply.");

  // Store all positions that are mate in `ply` ply in a new map.
  auto& new_map = data_.emplace_back();

  // Run over all positions that are mate in ply minus one.
  for (auto const& value : data_[ply - 1])
  {
    Board const& current_position(value.first);
    Dout(dc::notice, "Mate in " << (ply - 1) << " position:");
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
        auto ibp = new_map.try_emplace(board, ply);
        Dout(dc::preceding, "Predecing board:");
        Debug(ibp.first->first.debug_utf8art(dc::preceding));
      }
    }
    Dout(dc::preceding, " `---End of preceding positions.");
  }
}
