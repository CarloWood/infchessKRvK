#pragma once

#include "BlockIndex.h"
#include "BlockSquareCompact.h"
#include "utils/has_print_on.h"

// This class defines a print_on method.
using utils::has_print_on::operator<<;

class KingSquare
{
 public:
  static constexpr int bits = Size::block::square_bits + BlockIndex::number_of_blocks_bits;
  using coordinates_type = uint_type<bits>;
  static constexpr int available_bits = std::numeric_limits<coordinates_type>::digits;
  static constexpr coordinates_type mask = create_mask<coordinates_type, bits>();
  static constexpr int block_square_shift = BlockIndex::number_of_blocks_bits;
  static constexpr coordinates_type block_index_mask = create_mask<coordinates_type, BlockIndex::number_of_blocks_bits>();

  static constexpr BlockIndex xy_to_block_index(int x, int y) { return {x, y}; };
  static BlockSquareCompact xy_to_block_square(int x, int y)
  {
    return {
      static_cast<int>(static_cast<unsigned int>(x) % Size::block::x),
      static_cast<int>(static_cast<unsigned int>(y) % Size::block::y)
    };
  }

  static coordinates_type block_index_square_to_coordinates(BlockIndex block_index, BlockSquareCompact square)
  {
    return block_index.index() | (square.coordinates() << block_square_shift);
  }

 private:
  // <square_bits><block_index>
  // Note: if block_index is only 1 bit (only 1 or 2 blocks in the board) then
  // the least significant bit of <square_bits> will be used in the calculation
  // of block index FieldSpec::stride of the y-coordinate (namely, the x-stride
  // is a pre-shifted 1 as usual and the y-stride will be pre-shifted 2).
  coordinates_type coordinates_;

 public:
  KingSquare(coordinates_type square) : coordinates_(square)
  {
    // The unused most significant bits must be zero.
    ASSERT((coordinates_ & ~mask) == 0);
  }

  KingSquare(int x, int y) : coordinates_(block_index_square_to_coordinates(xy_to_block_index(x, y), xy_to_block_square(x, y))) { }

  // Accessors.
  BlockIndex block_index() const { return coordinates_ & block_index_mask; }
  BlockSquareCompact block_square() const { return coordinates_ >> block_square_shift; }

  int x_coord() const { return block_index().x_coord() + block_square().x_coord(); }
  int y_coord() const { return block_index().y_coord() + block_square().y_coord(); }

  // Used by the Board constructor.
  coordinates_type coordinates() const { return coordinates_; }

  friend bool operator==(KingSquare const& lhs, KingSquare const& rhs)
  {
    return lhs.coordinates_ == rhs.coordinates_;
  }

#ifdef CWDEBUG
  void sane_coordinates() const
  {
    int x = x_coord();
    int y = y_coord();
    ASSERT(0 <= x && x < Size::board::x);
    ASSERT(0 <= y && y < Size::board::y);
  }

  void print_on(std::ostream& os) const;
#endif
};
