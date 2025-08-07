#pragma once

#include "Classification.h"
#include "utils/log2.h"
#include <limits>
#include <concepts>

// The board is divided into `Blocks`.
// The block are numbered.
//
//             <------------------Px = 3 ------------------->
//     ^      .--------------+---------------+---------------.
//     |      |              |               |               | ^
//     |      | BlockIndex:  |               |               | |
//     |      |      9       |      10       |      11       | |
//     |      |              |               |               | |
//     |      +--------------+---------------+---------------+ |
//     |      |              |               |               | |
//     |      |              |               |               | |
//     |      |      6       |       7       |       8       | |
//     |      |              |               |               | | board_size_y
//  Py = 4    +--------------+---------------+---------------+ |/
//     |      |              |               |               | |
//     |      |              |               |               | |
//     |      |      3       |       4       |       5       | |
//     |      |              |               |               | |
//     |      +--------------+---------------+---------------+ |
//     |    ^ |__|           |               |               | |
//     | By=4 |__|           |               |               | |
//     |    | |__|___0_______|       1       |       2       | |
//     v    v |* |  |  |  |  |               |               | v
//           /+--------------+---------------+---------------+
//  corner__/  <---Bx=5----->
// * (0,0)     <----------------board_size_x---------------->

template<typename T>
struct integer_promotion
{
  static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "T must be an unsigned integral type");

  // The promoted type is what you get when you apply unary + to the type.
  using type = decltype(+std::declval<T>());
};

template<typename T>
using integer_promotion_t = typename integer_promotion<T>::type;

template<typename coordinates_type, int bits>
static constexpr coordinates_type create_mask()
{
  // The type that coordinates_type would be promoted to when shifting it.
  using mask_type = integer_promotion_t<coordinates_type>;
  // Create a mask with the same number of 1's as are bits in coordinates_type.
  mask_type const ones = std::numeric_limits<coordinates_type>::max();
  // The total number of (least significant) bits that are set in `ones`.
  int const total_bits = sizeof(coordinates_type) * CHAR_BIT;
  static_assert(0 < bits && bits <= total_bits, "The shift is out of range for coordinates_type!");
  mask_type const mask = ones >> (total_bits - bits);
  return mask;
}

template<unsigned int size_x, unsigned int size_y>
struct RectangleSize
{
  // The size of the corresponding rectangle.
  static constexpr unsigned int x = size_x;
  static constexpr unsigned int y = size_y;
  // The number of bits needed to store a single x coordinate.
  static constexpr int coord_bits_x = utils::ceil_log2(size_x);
  // The number of bits needed to store a single y coordinate.
  static constexpr int coord_bits_y = utils::ceil_log2(size_y);
  // A Square has two coordinates: x and y.
  static constexpr int square_bits = coord_bits_x + coord_bits_y;
  // Define the smallest type that can hold all required bits.
  using coordinates_type = uint_type<square_bits>;
  // Get the number of bits in `coordinates_type`.
  static constexpr int available_bits = std::numeric_limits<coordinates_type>::digits;

  // Lets store the coordinates of a Square as:
  //
  //   [ y-coord-bits ][ x-coord-bits ]
  //   <-coord_bits_y-><-coord_bits_x->
  //   <----------square_bits--------->
  //   00000000000000001111111111111111     <-- coord_mask_x
  //   11111111111111111111111111111111     <-- square_mask

  // Bit-mask for the x coordinate.
  static constexpr coordinates_type coord_mask_x = create_mask<coordinates_type, coord_bits_x>();
  // Bit-mask for a single square (two coordinates).
  static constexpr coordinates_type square_mask = create_mask<coordinates_type, square_bits>();

  static constexpr coordinates_type xy_to_coordinates(int x, int y)
  {
    //   [ y-coord-bits ][ x-coord-bits ]
    //                   <-coord_bits_x->
    return (y << coord_bits_x) | x;
  }

  // Extract the x-coordinate from a coordinates_type.
  static constexpr int x_coord(coordinates_type square_coordinates)
  {
    return square_coordinates & coord_mask_x;
  }

  // Extract the y-coordinate from a coordinates_type.
  static constexpr int y_coord(coordinates_type square_coordinates)
  {
    // The unused more significant bits must be zero.
    ASSERT((square_coordinates & ~square_mask) == 0);
    return square_coordinates >> coord_bits_x;
  }
};

template<class T>
concept RectangleSizeConcept = requires(T rectangle_size) {
  { RectangleSize{rectangle_size} } -> std::same_as<T>;
};

struct Size
{
 private:
  static constexpr unsigned int Bx = 8;         // Width in squares of one "king block".
  static constexpr unsigned int By = 8;         // Height in square of one "king block".
  static constexpr unsigned int Px = 2;
  static constexpr unsigned int Py = 2;

  static constexpr unsigned int board_size_x = Bx * Px;
  static constexpr unsigned int board_size_y = By * Py;

  static constexpr unsigned int max_ply_estimate = 256;
  static constexpr unsigned int max_number_of_children_estimate = board_size_x + board_size_y + 6; // Number of rook moves plus number of
                                                                                                   // king moves, including illegal ones.
  static constexpr int ply_bits = utils::ceil_log2(max_ply_estimate);
  static constexpr int number_of_children_bits = utils::ceil_log2(max_number_of_children_estimate);

 public:
  using block = RectangleSize<Bx, By>;                          // Size information for a Block.
  using board = RectangleSize<board_size_x, board_size_y>;      // Size information for a Board.

  using classification_type = Classification::bits_type;
  using ply_type = uint_type<ply_bits>;
  using number_of_children_type = uint_type<number_of_children_bits>;
};
