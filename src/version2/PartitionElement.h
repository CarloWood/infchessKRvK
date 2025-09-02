#pragma once

#include "BlackKingSquare.h"
#include "WhiteKingSquare.h"
#include "WhiteRookSquare.h"
#include "SquareCompact.h"
#include "utils/VectorIndex.h"

class PartitionElement;
using InfoIndex = utils::VectorIndex<PartitionElement>;

// A PartitionElement wraps a size_t that encodes the coordinates of three squares as:
//   [ unused ][      black-king-square       ][       white-king-square      ][      white-rook-square       ]
//               Size::block::                   Size::block::                   Size::board::
//             [ y-coord-bits ][ x-coord-bits ][ y-coord-bits ][ x-coord-bits ][ y-coord-bits ][ x-coord-bits ]
//             <---------square_bits----------><---------square_bits----------><---------square_bits---------->
//             <-coord_bits_y-><-coord_bits_x-><-coord_bits_y-><-coord_bits_x-><-coord_bits_y-><-coord_bits_x->
//   <------------------------------------------64 bits------------------------------------------------------->
//
// Both coord_bits_y and coord_bits_x are the minimum required width in bits.
//
struct PartitionElementBase
{
  static constexpr InfoIndex info_index(
      SquareCompact<Size::block> black_king, SquareCompact<Size::block> white_king, SquareCompact<Size::board> white_rook)
  {
    size_t encoded = black_king.coordinates();
        // Size::block:: [ y-coord-bits ][ x-coord-bits ]
        //               <---------square_bits---------->
    encoded <<= Size::block::square_bits;  //            <<============shift=============
    encoded |= white_king.coordinates();// Size::block:: [ y-coord-bits ][ x-coord-bits ]
                                           //            <---------square_bits---------->
    encoded <<= Size::board::square_bits;  //                                            <<============shift=============
    encoded |= white_rook.coordinates();   //                              Size::board:: [ y-coord-bits ][ x-coord-bits ]
                                           //                                            <---------square_bits---------->
    return InfoIndex{encoded};
  }
};

class PartitionElement : private PartitionElementBase
{
  static constexpr SquareCompact<Size::block> bks{Size::block::limit_y | Size::block::limit_x};
  static constexpr SquareCompact<Size::block> wks{Size::block::limit_y | Size::block::limit_x};
  static constexpr SquareCompact<Size::board> wrs{Size::board::limit_y | Size::board::limit_x};

 public:
  // This is the number of elements in a single Partition: one more than the largest value that will ever be stored in an InfoIndex.
  static constexpr size_t number_of_elements = info_index(bks, wks, wrs).get_value() + 1;

 private:
  InfoIndex index_;

 public:
  constexpr PartitionElement(InfoIndex info_index) : index_(info_index) { }
  constexpr PartitionElement(
      SquareCompact<Size::block> black_king,
      SquareCompact<Size::block> white_king,
      SquareCompact<Size::board> white_rook) : PartitionElement(info_index(black_king, white_king, white_rook)) { }

  // Constants used by the increment operator.
  static constexpr InfoIndex rook_limit_x{Size::board::limit_x};
  static constexpr InfoIndex rook_limit_y{Size::board::limit_y};
  static constexpr size_t rook_unit_y{size_t{1} << Size::board::coord_bits_x};
  static constexpr size_t rook_x_coord_mask{Size::board::mask_x};
  static constexpr size_t rook_y_coord_mask{Size::board::mask_y};
  static constexpr int white_king_shift{Size::board::square_bits};
  static constexpr InfoIndex wk_limit_x{Size::block::limit_x << white_king_shift};
  static constexpr InfoIndex wk_limit_y{Size::block::limit_y << white_king_shift};
  static constexpr size_t wk_unit_x{size_t{1} << white_king_shift};
  static constexpr size_t wk_unit_y{wk_unit_x << Size::block::coord_bits_x};
  static constexpr size_t wk_x_coord_mask{Size::block::mask_x << white_king_shift};
  static constexpr size_t wk_y_coord_mask{Size::block::mask_y << white_king_shift};
  static constexpr int black_king_shift{white_king_shift + Size::block::square_bits};
  static constexpr InfoIndex bk_limit_x{Size::block::limit_x << black_king_shift};
  static constexpr InfoIndex bk_limit_y{Size::block::limit_y << black_king_shift};
  static constexpr size_t bk_unit_x{size_t{1} << black_king_shift};
  static constexpr size_t bk_unit_y{bk_unit_x << Size::block::coord_bits_x};
  static constexpr size_t bk_x_coord_mask{Size::block::mask_x << black_king_shift};
  static constexpr size_t bk_y_coord_mask{Size::block::mask_y << black_king_shift};
  static constexpr InfoIndex end{number_of_elements};

  PartitionElement& operator++()
  {
    if ((index_ & rook_x_coord_mask) < rook_limit_x)    // Can the x-coordinate of the rook still be incremented?
      ++index_;
    else
    {
      index_ &= ~rook_x_coord_mask;                     // Reset the rook x-coordinate to zero.
      if ((index_ & rook_y_coord_mask) < rook_limit_y)  // Can the y-coordinate of the rook still be incremented?
        index_ += rook_unit_y;
      else
      {
        index_ &= ~rook_y_coord_mask;                   // Reset the rook y-coordinate to zero.
        if ((index_ & wk_x_coord_mask) < wk_limit_x)
          index_ += wk_unit_x;
        else
        {
          index_ &= ~wk_x_coord_mask;
          if ((index_ & wk_y_coord_mask) < wk_limit_y)
            index_ += wk_unit_y;
          else
          {
            index_ &= ~wk_y_coord_mask;
            if ((index_ & bk_x_coord_mask) < bk_limit_x)
              index_ += bk_unit_x;
            else
            {
              index_ &= ~bk_x_coord_mask;
              if ((index_ & bk_y_coord_mask) < bk_limit_y)
                index_ += bk_unit_y;
              else
                index_ = end;
            }
          }
        }
      }
    }
    return *this;
  }

  BlockSquareCompact black_king_square() const
  {
    return static_cast<Size::block::coordinates_type>(index_.get_value() >> (Size::block::square_bits + Size::board::square_bits));
  }
  BlockSquareCompact white_king_square() const
  {
    return static_cast<Size::block::coordinates_type>((index_.get_value() >> Size::board::square_bits) & Size::block::square_mask);
  }
  SquareCompact<Size::board> white_rook_square() const
  {
    return static_cast<Size::board::coordinates_type>(index_.get_value() & Size::board::square_mask);
  }

  // Accessor.
  constexpr operator InfoIndex() const { return index_; }

  friend bool operator!=(PartitionElement lhs, PartitionElement rhs) { return lhs.index_ != rhs.index_; }
};

