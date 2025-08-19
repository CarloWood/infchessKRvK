#pragma once

#include "BlackKingSquare.h"
#include "WhiteKingSquare.h"
#include "WhiteRookSquare.h"
#include "SquareCompact.h"
#include "utils/VectorIndex.h"

class PartitionElement;
using InfoIndex = utils::VectorIndex<PartitionElement>;

// A PartitionElement wraps an InfoIndex that is encoded as
// <black-king-square><white-king-square><white-rook-square>.
//
class PartitionElement
{
 private:
  InfoIndex index_;

  static constexpr InfoIndex info_index(
      SquareCompact<Size::block> black_king, SquareCompact<Size::block> white_king, SquareCompact<Size::board> white_rook)
  {
    size_t encoded = black_king.coordinates();

    encoded <<= Size::block::square_bits;
    encoded |= white_king.coordinates();

    encoded <<= Size::board::square_bits;
    encoded |= white_rook.coordinates();

    return InfoIndex{encoded};
  }

 public:
  constexpr PartitionElement(InfoIndex info_index) : index_(info_index) { }
  constexpr PartitionElement(
      SquareCompact<Size::block> black_king,
      SquareCompact<Size::block> white_king,
      SquareCompact<Size::board> white_rook) : PartitionElement(info_index(black_king, white_king, white_rook)) { }

  PartitionElement& operator++() { ++index_; return *this; }

  BlockSquareCompact black_king_square() const
  {
    return static_cast<Size::block::coordinates_type>(index_.get_value() >> (Size::block::square_bits + Size::board::square_bits));
  }
  BlockSquareCompact white_king_square() const
  {
    return static_cast<Size::block::coordinates_type>((index_.get_value() >> Size::block::square_bits) & Size::block::square_mask);
  }
  SquareCompact<Size::board> white_rook_square() const
  {
    return static_cast<Size::board::coordinates_type>(index_.get_value() & Size::board::square_mask);
  }

  // Accessor.
  constexpr operator InfoIndex() const { return index_; }

  friend bool operator!=(PartitionElement lhs, PartitionElement rhs) { return lhs.index_ != rhs.index_; }
};

