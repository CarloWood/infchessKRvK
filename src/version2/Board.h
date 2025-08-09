#pragma once

#include "Color.h"
#include "Square.h"
#include "utils/macros.h"
#include <cstdint>
#include <functional>
#include "debug.h"

#ifdef CWDEBUG
// This class defines a print_on method.
using utils::has_print_on::operator<<;
#endif

class Board
{
 public:
  static constexpr int encoded_bits = BlackKingSquare::bits + WhiteKingSquare::bits + WhiteRookSquare::bits;
  using encoded_type = uint_type<encoded_bits>;
  static constexpr int available_bits = std::numeric_limits<encoded_type>::digits;
  static constexpr int black_king_shift = WhiteRookSquare::bits + WhiteKingSquare::bits;
  static constexpr int white_king_shift = WhiteRookSquare::bits;
  static constexpr encoded_type black_king_mask = BlackKingSquare::mask;
  static constexpr encoded_type white_king_mask = WhiteKingSquare::mask;
  static constexpr encoded_type white_rook_mask = WhiteRookSquare::mask;

  // Maximum number of children or parents a node can have: the maximum occurs with white to move:
  // Size::board::x - 1 horizontal rook moves, Size::board::y - 1 vertical rook moves and 8 possible king moves.
  static constexpr unsigned int max_degree = Size::board::x + Size::board::y + 6;
  using neighbors_type = std::array<Board, max_degree>;

 private:
  encoded_type encoded_;        // Compact representation of <BlackKingSquare><WhiteKingSquare><WhiteRookSquare>.

  // Private constructor.
  Board(encoded_type encoded) : encoded_(encoded) { }

 public:
  Board(BlackKingSquare const& bk, WhiteKingSquare const& wk, WhiteRookSquare const& wr) :
    encoded_((encoded_type{bk.coordinates()} << black_king_shift) |
             (encoded_type{wk.coordinates()} << white_king_shift) |
              encoded_type{wr.coordinates()}) { }

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

  size_t as_index() const { return encoded_; }

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

  enum Neighbor {
    children,
    parents
  };

  template<color_type to_move>
  int get_king_moves(Neighbor direction, neighbors_type& neighbors_out);

  int get_neighbors(Color to_move, Neighbor direction, neighbors_type& neighbors_out);

  enum class Figure {
    none,
    black_king,
    white_king,
    white_rook
  };

  static void utf8art(std::ostream& os, Color to_move, bool xyz, std::function<Figure (Square)> select_figure);
  void utf8art(std::ostream& os, Color to_move, bool xyz = false) const;

#ifdef CWDEBUG
  // Allow printing a Board to an ostream.
  void print_on(std::ostream& os) const;

  // Print the board as utf8art with Dout.
  void debug_utf8art(libcwd::channel_ct const& debug_channel) const;
#endif

 public: //FIXME private:
  // Used by black_has_moves, determine_check, determine_draw and get_succeeding_boards.
  std::tuple<Square, Square, Square> abbreviations() const;

  enum FieldType
  {
    bkbi,       // BlackKingSquare::block_index().
    bkbc,       // BlackKingSquare::block_square().
    wkbi,       // WhiteKingSquare::block_index().
    wkbc,       // WhiteKingSquare::block_square().
    wr          // WhiteRookSquare::coordinates().
  };

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
    DoutEntering(dc::notice, "Board::inc_field<" << xy << ", " << ft << ">() with this = [" << *this << "]");

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
      Dout(dc::notice, "Returning false");
      return false;
    }
    encoded_ += fs.stride;
    Dout(dc::notice, "encoded_ = " << *this);
    return true;
  }

  template<int xy, FieldType ft>
  bool dec_field()
  {
    DoutEntering(dc::notice, "Board::dec_field<" << xy << ", " << ft << ">() with this = [" << *this << "]");

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
      Dout(dc::notice, "Returning false");
      return false;
    }
    encoded_ -= fs.stride;
    Dout(dc::notice, "encoded_ = " << *this);
    return true;
  }
};

template<color_type to_move>
int Board::get_king_moves(Neighbor direction, neighbors_type& neighbors_out)
{
  int neighbors = 0;

  Board neighbor(*this);

  // Store it in the output array.
  neighbors_out[neighbors++] = neighbor;

  return neighbors;
}
