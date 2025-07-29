#pragma once

#include "Square.h"
#include "Color.h"
#include "utils/has_print_on.h"
#include "utils/log2.h"
#include <cstdint>
#include <functional>
#include <tuple>
#include <type_traits>
#include "debug.h"

namespace version0 {

#ifdef CWDEBUG
// This class defines a print_on method.
using utils::has_print_on::operator<<;
#endif

class Board
{
public:
  enum class Mate : std::uint8_t
  {
    no,
    stalemate,
    yes
  };

 protected:
  int const board_size_;
  Square black_king_;
  Square white_king_;
  Square white_rook_;

  // Use this as: auto [bk, wk, wr] = Board::abbreviations();
  std::tuple<Square const&, Square const&, Square const&> abbreviations() const
  {
    return {black_king_, white_king_, white_rook_};
  }

 public:
  Board(int board_size, Square const& black_king, Square const& white_king, Square const& white_rook) :
    board_size_(board_size), black_king_(black_king), white_king_(white_king), white_rook_(white_rook) { }

  // Accessors.
  Square const& black_king() const { return black_king_; }
  Square const& white_king() const { return white_king_; }
  Square const& white_rook() const { return white_rook_; }
  int board_size() const { return board_size_; }

  bool distance_less(Board const& board) const
  {
    // This function is only used for ordering of a std::map.
    if (black_king_.distance_less(board.black_king_))
      return true;
    if (board.black_king_.distance_less(black_king_))
      return false;
    if (white_king_.distance_less(board.white_king_))
      return true;
    if (board.white_king_.distance_less(white_king_))
      return false;
    return white_rook_.distance_less(board.white_rook_);
  }

  bool operator==(Board const& other) const {
    return black_king_ == other.black_king_ && white_king_ == other.white_king_ && white_rook_ == other.white_rook_;
  };

  enum class Figure {
    none,
    black_king,
    white_king,
    white_rook
  };

  static void utf8art(std::ostream& os, int board_size, std::function<Figure (Square)> select_figure);
  void utf8art(std::ostream& os) const;

  void set_black_king_square(Square bk_pos)
  {
    black_king_ = bk_pos;
  }

  void set_white_king_square(Square wk_pos)
  {
    white_king_ = wk_pos;
  }

  void set_white_rook_square(Square wr_pos)
  {
    white_rook_ = wr_pos;
  }

#ifdef CWDEBUG
  // Allow printing a Board to an ostream.
  void print_on(std::ostream& os) const;

  // Print the board as utf8art with Dout.
  void debug_utf8art(libcwd::channel_ct const& debug_channel) const;
#endif

 public:
  bool black_has_moves() const;
  bool determine_check() const;
  // These are only called for legal positions:
  Mate determine_mate(Color to_move) const;     // Uses black_has_moves and determine_check.
  bool determine_draw(Color to_move) const;     // Uses determine_mate.
  bool determine_legal(Color to_move) const;    // Uses determine_check.

  std::vector<Board> get_succeeding_boards(Color to_move) const;

#if CW_DEBUG
 private:
  // By default, assume black is to move (only used for an assert).
  virtual bool black_is_to_move() const { return true; }
#endif
};

struct DistanceCompare
{
  bool operator()(Board const& lhs, Board const& rhs) const
  {
    return lhs.distance_less(rhs);
  }
};

} // namespace version0

namespace version1 {

#ifdef CWDEBUG
// This class defines a print_on method.
using utils::has_print_on::operator<<;
#endif

class Board
{
public:
  static constexpr unsigned int board_size = 5;
  // The number of bits needed to store a single coordinate (horizontal or vertical: the board is square).
  static constexpr int coord_bits = utils::ceil_log2(board_size);
  // A square has two coordinates: x and y.
  static constexpr int square_bits = 2 * coord_bits;
  // A board has three pieces: black king, white king and white rook.
  static constexpr int total_required_bits = 3 * square_bits;
  // Define the smallest type that can hold all required bits.
  using coordinates_type =
    std::conditional_t<total_required_bits <=  8, uint8_t,
    std::conditional_t<total_required_bits <= 16, uint16_t,
    std::conditional_t<total_required_bits <= 32, uint32_t,
    std::conditional_t<total_required_bits <= 64, uint64_t,
        void>>>>;
  static constexpr int available_bits = std::numeric_limits<coordinates_type>::digits;

  // Lets store the coordinates of a Square as:
  //
  //   [y-coord-bits][x-coord-bits]
  //   <-coord_bits-><-coord_bits->
  //   <--------square_bits------->
  //   0000000000000011111111111111     <-- coord_mask
  //   1111111111111111111111111111     <-- square_mask

  // Bit-mask for a single coordinate.
  static constexpr coordinates_type coord_mask = ~coordinates_type{0} >> (available_bits - coord_bits);
  // Bit-mask for a single square (two coordinates).
  static constexpr coordinates_type square_mask = ~coordinates_type{0} >> (available_bits - square_bits);

  // Lets store the piece coordinates in the order:
  //
  //   [black-king][white-king][white-rook]
  //   <-------total_required_bits-------->
  //               <---------------------->     <-- black_king_shift
  //                           <---------->     <-- white_king_shift

  static constexpr int black_king_shift = 2 * square_bits;      // Shift bits required for the white rook and king.
  static constexpr int white_king_shift = square_bits;          // Shift bits required for the white rook.

  // Type to store compact coordinates of a single square in.
  using square_coordinates_type = uint16_t;

  static constexpr coordinates_type white_rook_mask = square_mask;
  static constexpr coordinates_type white_king_mask = white_rook_mask << white_king_shift;
  static constexpr coordinates_type black_king_mask = white_rook_mask << black_king_shift;

  // Convert a Square to square_coordinates_type.
  static constexpr square_coordinates_type to_square_coordinates(Square const& square)
  {
    using namespace coordinates;
    // A square_coordinates_type contains <y-coord-bits><x-coord-bits>.
    square_coordinates_type square_coordinates = square[x];
    square_coordinates |= square[y] << coord_bits;
    return square_coordinates;
  }

  // Extract the x-coordinate from a square_coordinates_type.
  static constexpr int x_coord(square_coordinates_type square)
  {
    return square & coord_mask;
  }

  // Extract the y-coordinate from a square_coordinates_type.
  static constexpr int y_coord(square_coordinates_type square)
  {
    return (square >> coord_bits) & coord_mask;
  }

  // Convert a square_coordinates_type to a Square.
  static constexpr Square to_square(square_coordinates_type square)
  {
    return {x_coord(square), y_coord(square)};
  }

  enum class Mate : std::uint8_t
  {
    no,
    stalemate,
    yes
  };

 protected:
  coordinates_type coordinates_;        // The coordinates of the square of all three pieces, encoded as <black-king><white-king><white-rook>.

 public:
  Board(Square const& black_king, Square const& white_king, Square const& white_rook) :
    coordinates_(to_square_coordinates(black_king) << black_king_shift |
                 to_square_coordinates(white_king) << white_king_shift |
                 to_square_coordinates(white_rook)) { }

  // Accessors.
  // Convert Board piece to square_coordinates_type.
  square_coordinates_type black_king() const { return (coordinates_ >> black_king_shift) & square_mask; }
  square_coordinates_type white_king() const { return (coordinates_ >> white_king_shift) & square_mask; }
  square_coordinates_type white_rook() const { return coordinates_ & square_mask; }

  bool distance_less(Board const& board) const
  {
    return coordinates_ < board.coordinates_;
  }

  bool operator==(Board const& other) const {
    return coordinates_ == other.coordinates_;
  };

  enum class Figure {
    none,
    black_king,
    white_king,
    white_rook
  };

  static void utf8art(std::ostream& os, std::function<Figure (Square)> select_figure);
  void utf8art(std::ostream& os) const;

  void set_black_king_square(Square const& black_king)
  {
    coordinates_ &= ~black_king_mask;
    coordinates_ |= to_square_coordinates(black_king) << black_king_shift;
  }

  void set_white_king_square(Square const& white_king)
  {
    coordinates_ &= ~white_king_mask;
    coordinates_ |= to_square_coordinates(white_king) << white_king_shift;
  }

  void set_white_rook_square(Square const& white_rook)
  {
    coordinates_ &= ~white_rook_mask;
    coordinates_ |= to_square_coordinates(white_rook);
  }

#ifdef CWDEBUG
  // Allow printing a Board to an ostream.
  void print_on(std::ostream& os) const;

  // Print the board as utf8art with Dout.
  void debug_utf8art(libcwd::channel_ct const& debug_channel) const;
#endif

 public:
  bool kings_next_to_eachother() const
  {
    square_coordinates_type bk = black_king();
    square_coordinates_type wk = white_king();
    int bkx = x_coord(bk);
    int bky = y_coord(bk);
    int wkx = x_coord(wk);
    int wky = y_coord(wk);
    // Also return true if they occupy the same square.
    return std::abs(bkx - wkx) <= 1 && std::abs(bky - wky) <= 1;
  }

  bool black_has_moves() const;
  bool determine_check() const;
  // These are only called for legal positions:
  Mate determine_mate(Color to_move) const;     // Uses black_has_moves and determine_check.
  bool determine_draw(Color to_move) const;     // Uses determine_mate.
  bool determine_legal(Color to_move) const;    // Uses determine_check.

  std::vector<Board> get_succeeding_boards(Color to_move) const;

 protected:
  // Use this as: auto [bk, wk, wr] = Board::abbreviations();
  std::tuple<Square, Square, Square> abbreviations() const
  {
    return {to_square(black_king()), to_square(white_king()), to_square(white_rook())};
  }

#if CW_DEBUG
 private:
  // By default, assume black is to move (only used for an assert).
  virtual bool black_is_to_move() const { return true; }
#endif
};

struct DistanceCompare
{
  bool operator()(Board const& lhs, Board const& rhs) const
  {
    return lhs.distance_less(rhs);
  }
};

} // namespace version1
