#pragma once

#include "BlockIndex.h"
#include "BlockSquareCompact.h"

class KingSquare
{
 public:
  static constexpr int bits = BlockIndex::number_of_blocks_bits + Size::block::square_bits;
  using coordinates_type = uint_type<bits>;
  static constexpr int available_bits = std::numeric_limits<coordinates_type>::digits;
  static constexpr coordinates_type mask = create_mask<coordinates_type, bits>();
  static constexpr int block_index_shift = Size::block::square_bits;

  static constexpr BlockIndex xy_to_block_index(int x, int y) { return {x, y}; };
  static constexpr BlockSquareCompact xy_to_block_square(int x, int y)
  {
    return {
      static_cast<int>(static_cast<unsigned int>(x) % Size::block::x),
      static_cast<int>(static_cast<unsigned int>(y) % Size::block::y)
    };
  }

  static constexpr coordinates_type block_index_square_to_coordinates(BlockIndex block_index, BlockSquareCompact square)
  {
    return (block_index.index() << block_index_shift) | square.coordinates();
  }

 private:
  // <block_index><square_bits>
  coordinates_type coordinates_;

 public:
  KingSquare(coordinates_type square) : coordinates_(square)
  {
    // The unused most significant bits must be zero.
    ASSERT((coordinates_ & ~mask) == 0);
  }

  constexpr KingSquare(int x, int y) : coordinates_(block_index_square_to_coordinates(xy_to_block_index(x, y), xy_to_block_square(x, y))) { }

  // Accessors.
  BlockIndex block_index() const { return coordinates_ >> block_index_shift; }
  BlockSquareCompact block_square() const { return coordinates_ & Size::block::square_mask; }

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
#endif
};
