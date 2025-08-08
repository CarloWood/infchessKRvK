#pragma once

#include "Size.h"

class BlockIndex
{
 public:
  static constexpr unsigned int Px = Size::board::x / Size::block::x;                   // The width of the board in blocks.
  static constexpr unsigned int Py = Size::board::y / Size::block::y;                   // The height of the board in blocks.
  static constexpr unsigned int number_of_blocks = Px * Py;                             // Total number of blocks.
  static constexpr int number_of_blocks_bits = utils::ceil_log2(number_of_blocks);      // The number of bits required to store a BlockIndex.
  using index_type = uint_type<number_of_blocks_bits>;
  static constexpr index_type mask = create_mask<index_type, number_of_blocks_bits>();
  static constexpr index_type stride_x = index_type{1};
  static constexpr index_type stride_y = index_type{1} * Px;

  static constexpr index_type xy_to_index(int x, int y) { return (y / Size::block::y) * Px + (x / Size::block::x); }

 private:
  index_type index_;    // Index of a Block, see ASCII art in Size.h.

 public:
  BlockIndex(index_type index) : index_(index) { }
  constexpr BlockIndex(int x, int y) : index_(xy_to_index(x, y)) { }

  // Accessor.
  constexpr index_type index() const { return index_; }

  int x_coord() const { return (index_ % Px) * Size::block::x; }
  int y_coord() const { return (index_ / Px) * Size::block::y; }
};
