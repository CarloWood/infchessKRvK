#ifndef BOARD_H
#define BOARD_H

#include "Partition.h"
#include "PartitionElement.h"
#include "Square.h"
#include "../Color.h"
#include "utils/macros.h"
#include "utils/is_between.h"
#include "utils/VectorIndex.h"
#include <cstdint>
#include <functional>
#include "debug.h"

#ifdef CWDEBUG
// This class defines a print_on method.
using utils::has_print_on::operator<<;
#endif

// Forward declarations.
class Board;
class Graph;

class Board
{
 public:
  static constexpr int encoded_bits = BlackKingSquare::bits + WhiteKingSquare::bits + WhiteRookSquare::bits;
  using encoded_type = uint_type<encoded_bits>;
  static constexpr int total_bits = sizeof(encoded_type) * CHAR_BIT;
  static constexpr int black_king_shift = WhiteRookSquare::bits + WhiteKingSquare::bits;
  static constexpr int white_king_shift = WhiteRookSquare::bits;
  static constexpr encoded_type black_king_mask = BlackKingSquare::mask;
  static constexpr encoded_type white_king_mask = WhiteKingSquare::mask;
  static constexpr encoded_type white_rook_mask = WhiteRookSquare::mask;

  // Maximum number of children or parents a node can have: the maximum occurs with white to move:
  // Size::board::x - 1 horizontal rook moves, Size::board::y - 1 vertical rook moves and 8 possible king moves.
  static constexpr unsigned int max_degree = Size::board::x + Size::board::y + 6;
  using neighbors_type = std::array<Board, max_degree>;

  // The relationship between two positions where one can be reached from the other.
  // Where `position` means `Board` plus an externally provided `to_move`.
  enum Relation
  {
    children,           // Positions that can be reached from the current position.
    parents             // Positions from which the current position can be reached.
  };

  // Used as index into an array.
  enum Direction
  {
    North,              // Towards larger y-coordinate values.
    East,               // Towards larger x-coordinate values.
    South,              // Towards the bottom edge.
    West                // Towards the left edge.
  };

  enum FieldType
  {
    bkbi,               // BlackKingSquare::block_index().
    bkbc,               // BlackKingSquare::block_square().
    wkbi,               // WhiteKingSquare::block_index().
    wkbc,               // WhiteKingSquare::block_square().
    wr                  // WhiteRookSquare::coordinates().
  };

  // Used for printing a board.
  enum class Figure
  {
    none,
    black_king,
    white_king,
    white_rook,
    marker
  };

 private:
  encoded_type encoded_;        // Compact representation of <BlackKingSquare><WhiteKingSquare><WhiteRookSquare>.

  // Private constructor.
  Board(encoded_type encoded) : encoded_(encoded) { }

 public:
  Board() = default;
  Board(BlackKingSquare const& bk, WhiteKingSquare const& wk, WhiteRookSquare const& wr) :
    encoded_((encoded_type{bk.coordinates()} << black_king_shift) |
             (encoded_type{wk.coordinates()} << white_king_shift) |
              encoded_type{wr.coordinates()}) { }
  Board(Partition partition, PartitionElement partition_element) : Board(
      {KingSquare::block_index_square_to_coordinates(partition.black_king_block_index(), partition_element.black_king_square())},
      {KingSquare::block_index_square_to_coordinates(partition.white_king_block_index(), partition_element.white_king_square())},
      partition_element.white_rook_square().coordinates()) { }

  Board& operator=(encoded_type encoded)
  {
    encoded_ = encoded;
    return *this;
  }

  enum class Mate : std::uint8_t
  {
    no,
    stalemate,
    yes
  };

  bool black_has_moves() const;
  bool determine_check() const;
  // These are only called for legal positions:
  Mate determine_mate(Color to_move) const;     // Uses black_has_moves and determine_check.
  bool determine_draw(Color to_move) const;     // Uses determine_mate.
  bool determine_legal(Color to_move) const;    // Uses determine_check.

  BlackKingSquare black_king() const
  {
    encoded_type bk_square = encoded_ >> black_king_shift;
    // The unused higher significant bits shoud be zero.
    ASSERT((bk_square & ~black_king_mask) == 0);
    return static_cast<BlackKingSquare::coordinates_type>(bk_square);
  }

  WhiteKingSquare white_king() const
  {
    return static_cast<WhiteKingSquare::coordinates_type>((encoded_ >> white_king_shift) & white_king_mask);
  }

  WhiteRookSquare white_rook() const
  {
    return static_cast<WhiteRookSquare::coordinates_type>(encoded_ & white_rook_mask);
  }

  Partition as_partition() const { return {black_king().block_index(), white_king().block_index()}; }
  PartitionElement as_partition_element() const { return {black_king().block_square(), white_king().block_square(), white_rook()}; }

  // Convenience functions to extract the individual coordinates from the compact object returned by black_king.
  static auto x_coord(BlackKingSquare square) { return square.block_index().x_coord() + square.block_square().x_coord(); }
  static auto y_coord(BlackKingSquare square) { return square.block_index().y_coord() + square.block_square().y_coord(); }

  void set_black_king_square(BlackKingSquare black_king)
  {
    Debug(black_king.sane_coordinates());
    encoded_ &= ~(black_king_mask << black_king_shift);
    encoded_ |= black_king.coordinates() << black_king_shift;
  }

  void set_white_king_square(WhiteKingSquare white_king)
  {
    Debug(white_king.sane_coordinates());
    encoded_ &= ~(white_king_mask << white_king_shift);
    encoded_ |= white_king.coordinates() << white_king_shift;
  }

  void set_white_rook_square(WhiteRookSquare white_rook)
  {
    Debug(white_rook.sane_coordinates());
    encoded_ &= ~white_rook_mask;
    encoded_ |= white_rook.coordinates();
  }

  template<Relation relation, color_type to_move>
  void generate_king_moves(neighbors_type& neighbors_out, int& neighbors) const;
  template<Relation relation>
  void generate_rook_moves(neighbors_type& neighbors_out, int& neighbors) const;
  template<Relation relation, color_type to_move>
  int generate_neighbors(neighbors_type& neighbors_out) const;

  static void utf8art(std::ostream& os, Color to_move, bool xyz, std::function<Figure (Square)> select_figure);
  void utf8art(std::ostream& os, Color to_move, bool xyz = false, Square marker = Square{-1, -1}) const;

  std::string get_move(Board to_board) const;

  friend bool operator==(Board lhs, Board rhs) { return lhs.encoded_ == rhs.encoded_; }
  friend bool operator<(Board lhs, Board rhs) { return lhs.encoded_ < rhs.encoded_; }

#ifdef CWDEBUG
  // Test member function generate_neighbors.
  static void generate_neighbors_testsuite(Graph const& graph);

  // Allow printing a Board to an ostream.
  void print_on(std::ostream& os) const;

  // Print the board as utf8art with Dout.
  void debug_utf8art(libcwd::channel_ct const& debug_channel, Square marker = Square{-1, -1}) const;
#endif

 private:
  // Used by black_has_moves, determine_check, determine_draw and get_succeeding_boards.
  std::tuple<Square, Square, Square> abbreviations() const;

  static char const* field_type_to_str(FieldType ft)
  {
    switch (ft)
    {
      AI_CASE_RETURN(bkbi);
      AI_CASE_RETURN(bkbc);
      AI_CASE_RETURN(wkbi);
      AI_CASE_RETURN(wkbc);
      AI_CASE_RETURN(wr);
    }
    AI_NEVER_REACHED
  }

  friend std::ostream& operator<<(std::ostream& os, FieldType ft)
  {
    return os << Board::field_type_to_str(ft);
  }

  // Return three values as function of the inputs xy, and FieldType.
  struct FieldSpec
  {
    encoded_type mask;          // The bits of encoded_ that are used for this field.
    encoded_type limit;         // The largest allowed value of this field (using the same bits as mask).
                                // However, in the case this is the x-coordinate of bkbi/wkbi, the value
                                // must be multiplied with the current block y-coordinate plus one to get the
                                // first value larger than the largest allowed value of this field.
    encoded_type stride;        // The amount that has to be added to encoded_ in order to increment the field value by one.
                                // In the case of bkbi/wkbi this might use the least significant bit of the field above it (bkbc/wkbc).
  };

  template<int xy, FieldType ft>
  static consteval FieldSpec field_spec()
  {
    FieldSpec field_spec;
    constexpr bool y_coord = xy == coordinates::y;

    if constexpr (ft == bkbi)
    {
      field_spec.mask = BlockIndex::mask << black_king_shift;
      field_spec.limit = (y_coord ? BlockIndex::number_of_blocks - 1 : BlockIndex::Px) << black_king_shift;
      field_spec.stride = (y_coord ? BlockIndex::stride_y : BlockIndex::stride_x) << black_king_shift;
    }
    else if constexpr (ft == bkbc)
    {
      constexpr int block_square_shift = KingSquare::block_square_shift + black_king_shift;
      field_spec.mask   = (y_coord ? Size::block::mask_y   : Size::block::mask_x)   << block_square_shift;
      field_spec.limit  = (y_coord ? Size::block::limit_y  : Size::block::limit_x)  << block_square_shift;
      field_spec.stride = (y_coord ? Size::block::stride_y : Size::block::stride_x) << block_square_shift;
    }
    else if (ft == wkbi)
    {
      field_spec.mask = BlockIndex::mask << white_king_shift;
      field_spec.limit = (y_coord ? BlockIndex::number_of_blocks - 1 : BlockIndex::Px) << white_king_shift;
      field_spec.stride = (y_coord ? BlockIndex::stride_y : BlockIndex::stride_x) << white_king_shift;
    }
    else if constexpr (ft == wkbc)
    {
      constexpr int block_square_shift = KingSquare::block_square_shift + white_king_shift;
      field_spec.mask   = (y_coord ? Size::block::mask_y   : Size::block::mask_x)   << block_square_shift;
      field_spec.limit  = (y_coord ? Size::block::limit_y  : Size::block::limit_x)  << block_square_shift;
      field_spec.stride = (y_coord ? Size::block::stride_y : Size::block::stride_x) << block_square_shift;
    }
    else if constexpr (ft == wr)
    {
      field_spec.mask   = y_coord ? Size::board::mask_y   : Size::board::mask_x;
      field_spec.limit  = y_coord ? Size::board::limit_y  : Size::board::limit_x;
      field_spec.stride = y_coord ? Size::board::stride_y : Size::board::stride_x;
    }
    return field_spec;
  }

  template<int xy, FieldType ft>
  bool inc_field()
  {
    //DoutEntering(dc::notice, "Board::inc_field<" << xy << ", " << ft << ">() with this = [" << *this << "]");

    constexpr FieldSpec fs = field_spec<xy, ft>();
    // In the case of a block index' x-coordinate, the limit is given as:
    //               __ actual limit.
    //              /
    // ⋮ ⋮   ⋮   ⋮ v ⋮
    //   +---+---+---+
    // 2 | 6 | 7 | 8 |
    //   +---+---+---+
    // 1 | 3 | 4 | 5 |
    //   +---+---+---+
    // 0 | 0 | 1 | 2 | 3 = fs.limit
    // ^ `---+---+---'
    // |   0   1   2
    // |   `---.---'
    // |       |
    // | x-coordinate
    // y-coordinate
    encoded_type limit = fs.limit;
    if constexpr ((ft == bkbi || ft == wkbi) && xy == coordinates::x)
    {
      KingSquare king_square = ft == bkbi ? static_cast<KingSquare const&>(black_king()) : static_cast<KingSquare const&>(white_king());
      auto block_y = king_square.block_index().index() / BlockIndex::Px;
      limit = fs.limit * block_y + (fs.limit - fs.stride);
    }
    if ((encoded_ & fs.mask) + fs.stride > limit)
    {
      //Dout(dc::notice, "Returning false");
      return false;
    }
    encoded_ += fs.stride;
    //Dout(dc::notice, "encoded_ = " << *this);
    return true;
  }

  template<int xy, FieldType ft>
  bool dec_field()
  {
    //DoutEntering(dc::notice, "Board::dec_field<" << xy << ", " << ft << ">() with this = [" << *this << "]");

    constexpr FieldSpec fs = field_spec<xy, ft>();
    encoded_type limit = 0;
    if constexpr ((ft == bkbi || ft == wkbi) && xy == coordinates::x)
    {
      encoded_type shifted_block_index = encoded_ & fs.mask;
      encoded_type shifted_block_y = (shifted_block_index / BlockIndex::Px) & fs.mask;
      limit = BlockIndex::Px * shifted_block_y;
    }
    // Test if we can subtract fs.stride from the current block index (encoded_ & fs.mask) without becoming less than limit.
    if ((encoded_ & fs.mask) < limit + fs.stride)
    {
      //Dout(dc::notice, "Returning false");
      return false;
    }
    encoded_ -= fs.stride;
    //Dout(dc::notice, "encoded_ = " << *this);
    return true;
  }

  template<int xy, FieldType ft>
  void reset_low_field()
  {
    static_assert(ft != bkbi && ft != wkbi, "This function doesn't work for block-index coordinates.");
    constexpr FieldSpec fs = field_spec<xy, ft>();
    encoded_ &= ~fs.mask;
  }

  template<int xy, FieldType ft>
  void reset_high_field()
  {
    static_assert(ft != bkbi && ft != wkbi, "This function doesn't work for block-index coordinates.");
    constexpr FieldSpec fs = field_spec<xy, ft>();
    encoded_ &= ~fs.mask;
    encoded_ |= fs.limit;
  }

  template<int xy, color_type color>
  bool inc_king()
  {
    constexpr FieldType kbc = color == black ? bkbc : wkbc;
    constexpr FieldType kbi = color == black ? bkbi : wkbi;

    // Try to step within the current block.
    if (inc_field<xy, kbc>())
      return true;

    // We are at the edge of the current block; increment the block index.
    if (!inc_field<xy, kbi>())
      return false;

    // Reset the block coordinate to zero.
    reset_low_field<xy, kbc>();
    return true;
  }

  template<int xy, color_type color>
  bool dec_king()
  {
    constexpr FieldType kbc = color == black ? bkbc : wkbc;
    constexpr FieldType kbi = color == black ? bkbi : wkbi;

    // Try to step within the current block.
    if (dec_field<xy, kbc>())
      return true;

    // We are at the edge of the current block; decrement the block index.
    if (!dec_field<xy, kbi>())
      return false;

    // Reset the block coordinate to the block size.
    reset_high_field<xy, kbc>();
    return true;
  }

  template<color_type color>
  BlockIndex block_index() const
  {
    constexpr int index_shift = color == black ? black_king_shift : white_king_shift;
    return (encoded_ >> index_shift) & BlockIndex::mask;
  }

  template<color_type color>
  BlockSquareCompact block_square() const
  {
    constexpr int block_square_shift = (color == black ? black_king_shift : white_king_shift) + KingSquare::block_square_shift;
    return (encoded_ >> block_square_shift) & Size::block::square_mask;
  }
};

#include "Square.h"
#endif // BOARD_H

#ifndef BOARD_defs_H
#define BOARD_defs_H

// Extract compact square coordinates into separate x and y coordinates for all three pieces.
inline std::tuple<Square, Square, Square> Board::abbreviations() const
{
  return {black_king(), white_king(), white_rook()};
}

template<Board::Relation relation, color_type to_move>
void Board::generate_king_moves(neighbors_type& neighbors_out, int& neighbors) const
{
  using namespace coordinates;

  // Get the coordinates of all pieces.
  auto const [bk, wk, wr] = abbreviations();

  // Assume the current king is at `k`, and the enemy king is on A through P, or somewhere else:
  //        .---+---+---+---+---+---+---+---.
  //        |   |   |   |   |   |   |   |   |
  //        +---+---+---+---+---+---+---+---+
  //        |   |   |   |   |   |   |   |   |
  //        +---+---+---+---+---+---+---+---+
  // dy = 2 |   | A | B | C | D | E |   |   |
  //        +---+---+---+---+---+---+---+---+
  //      1 |   | F |   |   |   | G |   |   |
  //        +---+---+---+---+---+---+---+---+
  //      0 |   | H |   | k |   | I |   |   |
  //        +---+---+---+---+---+---+---+---+
  //     -1 |   | J |   |   |   | K |   |   |
  //        +---+---+---+---+---+---+---+---+
  //     -2 |   | L | M | N | O | P |   |   |
  //        +---+---+---+---+---+---+---+---+
  //        |   |   |   |   |   |   |   |   |
  //        `---+---+---+---+---+---+---+---'
  //        dx = -2  -1   0   1   2
  //
  //
  //---------------------------------------------------------------------------
  // Calculate the difference between the coordinates of the two kings as: the other king minus the king that is to move.
  // We encode these deltas as 'delta + 2' stored in an unsigned int.
  std::array<unsigned int, 2> xy_encoded_delta;
  if constexpr (to_move == black)
    xy_encoded_delta = { static_cast<unsigned int>(wk[x] - bk[x] + 2), static_cast<unsigned int>(wk[y] - bk[y] + 2) };
  else
    xy_encoded_delta = { static_cast<unsigned int>(bk[x] - wk[x] + 2), static_cast<unsigned int>(bk[y] - wk[y] + 2) };

  // Next construct an uint64_t bit-mask with a single bit set iff the enemy king
  // is on one of the squares A through P, encoded as an 8x8 square as follows:
  //
  // msb
  //  |
  //  v 43210 <-- xy_encoded_delta[x]
  //  00000000
  //  00000000
  //  00EDCBA0 4
  //  00G000F0 3
  //  00I0k0H0 2
  //  00K000J0 1
  //  00PONML0 0
  //  00000000
  //         ^ ^
  //         | `-- xy_encoded_delta[y]
  //        lsb
  //
  // All the squares around L.
  constexpr uint64_t blocked_by_L = 0b\
00000000\
00000000\
00000000\
00000000\
00000111\
00000101\
00000111;

  // Calculate the squares that are blocked by the enemy king.
  uint64_t blocked_squares =
    (xy_encoded_delta[x] <= 4U && xy_encoded_delta[y] <= 4U)                    // If the enemy king is on A through P,
        ? blocked_by_L << (xy_encoded_delta[x] + 8 * xy_encoded_delta[y])       // encode the deltas.
        : 0;                                                                    // Otherwise, if the enemy king is far away, use zero.

  // These masks are used to mark squares attacked by the white rook, relative to the king that is moving.
  constexpr uint64_t west_of_king = 0b\
00000000\
00000000\
00000100\
00000100\
00000100\
00000000\
00000000;
  constexpr uint64_t south_of_king = 0b\
00000000\
00000000\
00000000\
00000000\
00011100\
00000000\
00000000;
  constexpr uint64_t south_east_of_king = 0b\
00000000\
00000000\
00000000\
00000000\
00010000\
00000000\
00000000;
  constexpr uint64_t south_west_of_king = 0b\
00000000\
00000000\
00000000\
00000000\
00000100\
00000000\
00000000;
  constexpr uint64_t north_west_of_king = 0b\
00000000\
00000000\
00000100\
00000000\
00000000\
00000000\
00000000;

  // Calculate the difference between the coordinates of the king and the rook as: the rook minus the king that is to move.
  // We encode these deltas as 'delta + 1' stored in an unsigned int.
  if constexpr (to_move == black)
    xy_encoded_delta = { static_cast<unsigned int>(wr[x] - bk[x] + 1), static_cast<unsigned int>(wr[y] - bk[y] + 1) };
  else
    xy_encoded_delta = { static_cast<unsigned int>(wr[x] - wk[x] + 1), static_cast<unsigned int>(wr[y] - wk[y] + 1) };
  // Calculate the square that the white rook occupies.
  uint64_t rook_square =
    (xy_encoded_delta[x] <= 2U && xy_encoded_delta[y] <= 2U)                          // If the rook is next to the king,
        ? south_west_of_king << (xy_encoded_delta[x] + 8 * xy_encoded_delta[y])       // encode the deltas.
        : 0;                                                                          // Otherwise, if the rook is far away, use zero.

  //---------------------------------------------------------------------------
  // Calculate the squares that are blocked by the rook.
  uint64_t rook_blocked_squares;
  if constexpr (to_move == black && relation == children)
  {
    // Calculate the difference between the coordinates of the king and the rook as: the rook minus the (black) king.
    // We encode these deltas as 'delta + 1' stored in an unsigned int.
    xy_encoded_delta = { static_cast<unsigned int>(wr[x] - bk[x] + 1), static_cast<unsigned int>(wr[y] - bk[y] + 1) };

    // Calculate the squares that are blocked by the white rook.
    rook_blocked_squares =
      (xy_encoded_delta[x] <= 2U &&                                             // If the rook is horizontally near the king,
       !(wk[x] == wr[x] && utils::is_between_le_lt(bk[y], wk[y], wr[y])))       // and the white king is not blocking the rook,
          ? west_of_king << xy_encoded_delta[x]                                 // encode the deltas.
          : 0;                                                                  // Otherwise, if the rook is horizontally far away, use zero.
    // Calculate the squares that are blocked by the white rook.
    rook_blocked_squares |=
      (xy_encoded_delta[y] <= 2U &&                                             // If the rook is vertically near the king,
       !(wk[y] == wr[y] && utils::is_between_le_lt(bk[x], wk[x], wr[x])))       // and the white king is not blocking the rook,
          ? south_of_king << (8 * xy_encoded_delta[y])                          // encode the deltas.
          : 0;                                                                  // Otherwise, if the rook is vertically far away, use zero.

    // The black king can take the white rook.
    rook_blocked_squares &= ~rook_square;
  }
  else
  {
    // If relation is children, then white is to move and the white king can't go where the white rook is.
    // If relation is parents, then irrespective of who is to move, the king can't come from the square where the rook is.
    rook_blocked_squares = rook_square;

    if constexpr (to_move == white && relation == parents)
    {
      bool bk_wr_same_file = bk[x] == wr[x];
      bool bk_wr_same_rank = bk[y] == wr[y];

      // We shouldn't generate parent positions that are illegal: do not return positions where black is in check.
      //
      // If the black king and the white rook are not on one line in the child position then we know that the parent
      // position (which only has the white king on a different square) can not possibly have been check.
      //
      // If the black king and the white rook are on the same square then the white rook is no longer on the board
      // and the white king has no restriction with regard to checks.
      if (bk_wr_same_file != bk_wr_same_rank)
      {
        // Calculate the difference between the coordinates of the king and the rook as: the rook minus the (white) king.
        // We encode these deltas as 'delta + 1' stored in an unsigned int.
        xy_encoded_delta = { static_cast<unsigned int>(wr[x] - wk[x] + 1), static_cast<unsigned int>(wr[y] - wk[y] + 1) };

        // Otherwise we must make sure that the white king is inbetween the black king and the white rook, in the parent position.
        if (bk_wr_same_rank)                            // Are the black king and the white rook on the same rank?
        {
          int min_x = std::min(bk[x], wr[x]);
          int max_x = std::max(bk[x], wr[x]);
          if (xy_encoded_delta[y] > 2 || min_x > wk[x] || wk[x] > max_x)
            return;                                     // No parent positions exist that lead to the current position with a king move.
          uint64_t valid_south_of_king = south_of_king;
          // At this point there is still the possibility that the white king has the same x-coordinate as the white rook, in
          // which case not all `south_of_king` bits can be used.
          if (wk[x] == wr[x])
          {
            // For example,
            // 2 ┃   ♚   ·   ·   ·   ·
            // 1 ┃ · ♜ · ♔ ·   ·   ·   ·
            //   ┗━a━b━c━d━e━f━g━h━i━j━k
            //
            // In this case a1 is not a square where the white king could have come from
            // because then black would have been in check with white to move. We still
            // need to consider c1 however.
            valid_south_of_king = wr[x] < bk[x] ? south_east_of_king : south_west_of_king;
          }
          uint64_t between = valid_south_of_king << (8 * xy_encoded_delta[y]);
          rook_blocked_squares |= ~between;
        }
        else // bk_wr_same_file                         // The black king and the white rook are on the same file.
        {
          int min_y = std::min(bk[y], wr[y]);
          int max_y = std::max(bk[y], wr[y]);
          if (xy_encoded_delta[x] > 2 || min_y > wk[y] || wk[y] > max_y)
            return;                                     // No parent positions exist that lead to the current position with a king move.
          uint64_t valid_west_of_king = west_of_king;
          // At this point there is still the possibility that the white king has the same y-coordinate as the white rook, in
          // which case not all `west_of_king` bits can be used.
          if (wk[y] == wr[y])
            valid_west_of_king = wr[y] < bk[y] ? north_west_of_king : south_west_of_king;
          uint64_t between = valid_west_of_king << xy_encoded_delta[x];
          rook_blocked_squares |= ~between;             // Then in the parent position, the white king must be inbetween them.
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  // Add the squares that are blocked by the rook.
  blocked_squares |= rook_blocked_squares;

  // Prepare the four positions where the king steps North, East, South and West.
  std::array<Board, 4> neighbor = {{ {*this}, {*this}, {*this}, {*this} }};

  // The block-index and block-coordinates field types of the king that is to move.
  constexpr int kbi = to_move == black ? bkbi : wkbi;
  constexpr int kbc = to_move == black ? bkbc : wkbc;

  // Attempt to calculate the positions that we get by stepping North, East, South or West with the king.
  std::array<bool, 4> success = {
    neighbor[North].inc_king<y, to_move>(),
    neighbor[East].inc_king<x, to_move>(),
    neighbor[South].dec_king<y, to_move>(),
    neighbor[West].dec_king<x, to_move>()
  };

  // Constants encoding the eight directions the king can step into.
  // Note that this representation is horizontally flipped.
  //
  // msb
  //  |
  //  v 43210 <-- xy_encoded_delta[x]
  //  00000000
  //  00000000
  //  00000000 4
  //  0000N000 3            ---.
  //  000EkW00 2            ---+-------.
  //  0000S000 1            ---+-------+-------.
  //  00000000 0               |       |       |
  //  00000000                 v       v       v
  //         ^ ^
  //         | `-- xy_encoded_delta[y]
  //        lsb
  //                    ...         000EkW00        00000000
  //                        0000N000        0000S000        00000000
  constexpr uint64_t N  = 0b0000100000000000000000000000000000000000;
  constexpr uint64_t E  = 0b0000000000010000000000000000000000000000;
  constexpr uint64_t S  = 0b0000000000000000000010000000000000000000;
  constexpr uint64_t W  = 0b0000000000000100000000000000000000000000;
  constexpr uint64_t NE = 0b0001000000000000000000000000000000000000;
  constexpr uint64_t NW = 0b0000010000000000000000000000000000000000;
  constexpr uint64_t SE = 0b0000000000000000000100000000000000000000;
  constexpr uint64_t SW = 0b0000000000000000000001000000000000000000;

  // If the step was successful (we didn't step outside the board) and now we're not next to the other king
  // or (if we're black) we put ourself in check, add the result to the output array.
  if (success[North])                   // Did not step off the board?
  {
    if (!(blocked_squares & N))         // Did not step next to the other king or put ourself in check?
      neighbors_out[neighbors++] = neighbor[North];
    // Next step West and East to reach North-West and North-East.
    Board neighbor_north_east(neighbor[North]);
    Board neighbor_north_west(neighbor[North]);
    bool success_north_east = neighbor_north_east.inc_king<x, to_move>();
    bool success_north_west = neighbor_north_west.dec_king<x, to_move>();
    // Add the new positions if we didn't step off the board nor next to the enemy king or put ourself in check.
    if (success_north_east && !(blocked_squares & NE))
      neighbors_out[neighbors++] = neighbor_north_east;
    if (success_north_west && !(blocked_squares & NW))
      neighbors_out[neighbors++] = neighbor_north_west;
  }
  if (success[East])
  {
    if (!(blocked_squares & E))
      neighbors_out[neighbors++] = neighbor[East];
  }
  if (success[South])
  {
    if (!(blocked_squares & S))
      neighbors_out[neighbors++] = neighbor[South];
    // Next step West and East to reach North-West and North-East.
    Board neighbor_south_east(neighbor[South]);
    Board neighbor_south_west(neighbor[South]);
    bool success_south_east = neighbor_south_east.inc_king<x, to_move>();
    bool success_south_west = neighbor_south_west.dec_king<x, to_move>();
    // Add the new positions if we didn't step off the board nor next to the enemy king or put ourself in check.
    if (success_south_east && !(blocked_squares & SE))
      neighbors_out[neighbors++] = neighbor_south_east;
    if (success_south_west && !(blocked_squares & SW))
      neighbors_out[neighbors++] = neighbor_south_west;
  }
  if (success[West])
  {
    if (!(blocked_squares & W))
      neighbors_out[neighbors++] = neighbor[West];
  }
}

template<Board::Relation relation>
void Board::generate_rook_moves(neighbors_type& neighbors_out, int& neighbors) const
{
  // Get the current x and y coordinates of the white rook and the white king.
  int bkx = black_king().x_coord();
  int bky = black_king().y_coord();
  int wkx = white_king().x_coord();
  int wky = white_king().y_coord();
  int wrx = white_rook().x_coord();
  int wry = white_rook().y_coord();

  // Calculate the distance to the board edge for each direction.
  std::array<unsigned int, 4> steps = {
    Size::board::y - 1 - wry,           // The number of squares North of the white rook.
    Size::board::x - 1 - wrx,           // The number of squares East of the white rook.
    static_cast<unsigned int>(wry),     // The number of squares South of the white rook.
    static_cast<unsigned int>(wrx)      // The number of squares West of the white rook.
  };
  // Correct these distances for a potential block by the white king.
  if (wrx == wkx)
  {
    // Only one coordinate can be the same.
    ASSERT(wky != wry);
    if (wky > wry)      // Is the king North of the rook?
      steps[North] = wky - wry - 1;
    else                // The king is South of the rook.
      steps[South] = wry - wky - 1;
  }
  else if (wry == wky)
  {
    // Only one coordinate can be the same.
    ASSERT(wkx != wrx);
    if (wkx > wrx)      // Is the king East of the rook?
      steps[East] = wkx - wrx - 1;
    else                // The king is West of the rook.
      steps[West] = wrx - wkx - 1;
  }

  for (int dir = North; dir <= West; ++dir)
  {
    Board neighbor(*this);                      // Start at the original position.
    if constexpr (relation == children)
    {
      // Move the white rook steps[dir] in the direction dir.
      switch (dir)
      {
        case North:
          for (unsigned int step = 0; step < steps[dir]; ++step)
          {
            neighbor.encoded_ += field_spec<1, wr>().stride;
            neighbors_out[neighbors++] = neighbor;
          }
          break;
        case East:
          for (unsigned int step = 0; step < steps[dir]; ++step)
          {
            neighbor.encoded_ += field_spec<0, wr>().stride;
            neighbors_out[neighbors++] = neighbor;
          }
          break;
        case South:
          for (unsigned int step = 0; step < steps[dir]; ++step)
          {
            neighbor.encoded_ -= field_spec<1, wr>().stride;
            neighbors_out[neighbors++] = neighbor;
          }
          break;
        case West:
          for (unsigned int step = 0; step < steps[dir]; ++step)
          {
            neighbor.encoded_ -= field_spec<0, wr>().stride;
            neighbors_out[neighbors++] = neighbor;
          }
          break;
      }
    }
    else
    {
      // The step at which black is in check.
      int check_step = -1;      // Use a default of -1 for the case that there is no check because the white king will block it.
      // Move the white rook steps[dir] in the direction dir.
      switch (dir)
      {
        case North:
          if (wrx == bkx && !(wkx == bkx && utils::is_between_le_lt(bky, wky, wry)))
            break;
          if (!(wky == bky && utils::is_between_le_lt(bkx, wkx, wrx)))  // Doesn't the white king block the check?
            check_step = bky - 1 - wry;
          for (unsigned int step = 0; step < steps[dir]; ++step)
          {
            neighbor.encoded_ += field_spec<1, wr>().stride;
            if (step != check_step)
              neighbors_out[neighbors++] = neighbor;
          }
          break;
        case East:
          if (wry == bky && !(wky == bky && utils::is_between_le_lt(bkx, wkx, wrx)))
            break;
          if (!(wkx == bkx && utils::is_between_le_lt(bky, wky, wry)))  // Doesn't the white king block the check?
            check_step = bkx - 1 - wrx;
          for (unsigned int step = 0; step < steps[dir]; ++step)
          {
            neighbor.encoded_ += field_spec<0, wr>().stride;
            if (step != check_step)
              neighbors_out[neighbors++] = neighbor;
          }
          break;
        case South:
          if (wrx == bkx && !(wkx == bkx && utils::is_between_le_lt(bky, wky, wry)))
            break;
          if (!(wky == bky && utils::is_between_le_lt(bkx, wkx, wrx)))  // Doesn't the white king block the check?
            check_step = wry - bky - 1;
          for (unsigned int step = 0; step < steps[dir]; ++step)
          {
            neighbor.encoded_ -= field_spec<1, wr>().stride;
            if (step != check_step)
              neighbors_out[neighbors++] = neighbor;
          }
          break;
        case West:
          if (wry == bky && !(wky == bky && utils::is_between_le_lt(bkx, wkx, wrx)))
            break;
          if (!(wkx == bkx && utils::is_between_le_lt(bky, wky, wry)))  // Doesn't the white king block the check?
            check_step = wrx - bkx - 1;
          for (unsigned int step = 0; step < steps[dir]; ++step)
          {
            neighbor.encoded_ -= field_spec<0, wr>().stride;
            if (step != check_step)
              neighbors_out[neighbors++] = neighbor;
          }
          break;
      }
    }
  }
}

template<Board::Relation relation, color_type to_move>
int Board::generate_neighbors(neighbors_type& neighbors_out) const
{
  // Do not call generate_neighbors after the white rook was already captured.
  ASSERT(Square{black_king()} != Square{white_rook()});
  int neighbors = 0;
  generate_king_moves<relation, to_move>(neighbors_out, neighbors);
  if (to_move == white)
    generate_rook_moves<relation>(neighbors_out, neighbors);
  return neighbors;
}

#endif // BOARD_defs_H
