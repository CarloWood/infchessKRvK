#pragma once

#include "Size.h"
#include "debug.h"

constexpr void constexpr_assert(bool condition, const char* message = "assertion failed") {
    if (!condition) {
        if (std::is_constant_evaluated()) {
            // During constant evaluation, this will make the function non-constexpr.
            // The compiler will reject it if used in constant expression context.
            throw std::logic_error(message);
        } else {
            // Runtime: print message and abort.
            std::fprintf(stderr, "Assertion failed: %s\n", message);
            std::abort();
        }
    }
}

// A SquareCompact wraps an unsigned integral type that encodes the
// coordinates of the square as:
//   [ unused ][ y-coord-bits ][ x-coord-bits ].
//             <-coord_bits_y-><-coord_bits_x->         Both coord_bits_y and coord_bits_x are the minimum required width in bits.
//   <---------------total_bits--------------->         (nearest power of two (8, 16, 32 or 64))
//
template<RectangleSizeConcept RectangleSize>
class SquareCompact
{
 public:
  using coordinates_type = typename RectangleSize::coordinates_type;

 private:
  coordinates_type coordinates_{};      // Uses RectangleSize::square_bits bits, with mask RectangleSize::square_mask.

 protected:
  SquareCompact() = default;

 public:
  constexpr SquareCompact(coordinates_type coordinates) : coordinates_(coordinates)
  {
    // The unused most significant bits must be zero.
    ASSERT((coordinates_ & ~RectangleSize::square_mask) == 0);
  }

  SquareCompact(int x, int y) : coordinates_(RectangleSize::xy_to_coordinates(x, y))
  {
    // The unused most significant bits must be zero.
    ASSERT((coordinates_ & ~RectangleSize::square_mask) == 0);
  }

  // Accessor.
  constexpr coordinates_type coordinates() const { return coordinates_; }

  int x_coord() const { return RectangleSize::x_coord(coordinates_); }
  int y_coord() const { return RectangleSize::y_coord(coordinates_); }

  // Compare equal.
  friend bool operator==(SquareCompact const& lhs, SquareCompact const& rhs)
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
