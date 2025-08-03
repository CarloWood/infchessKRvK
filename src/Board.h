#pragma once

#include "Color.h"
#include "utils/has_print_on.h"
#include "utils/log2.h"
#include <cstdint>
#include <functional>
#include <tuple>
#include <type_traits>
#include <limits>
#include "debug.h"

#ifdef CWDEBUG
// This class defines a print_on method.
using utils::has_print_on::operator<<;
#endif

class Board;

class SquareCompact
{
 public:
  // Type to store compact coordinates of a single square in.
  using coordinates_type = uint16_t;

 protected:
  // The x and y coordinate of this square, stored as
  //   [y-coord-bits][x-coord-bits]
  //   <-coord_bits-><-coord_bits->
  // where coord_bits is defined below, in class Board.
  // (0, 0) means the piece is in the (only) corner of the quarter-infinite board.
  coordinates_type coordinates_;

 protected:
  SquareCompact() = default;

 public:
  constexpr SquareCompact(coordinates_type coordinates) : coordinates_(coordinates) { }
  inline constexpr SquareCompact(int x, int y);

 public:
  constexpr coordinates_type coordinates() const
  {
    return coordinates_;
  }

  friend bool operator==(SquareCompact lhs, SquareCompact rhs)
  {
    return lhs.coordinates_ == rhs.coordinates_;
  }

#ifdef CWDEBUG
  void sane_coordinates(int board_size) const;
#endif
};

// Forward declare Square, because that includes this header (required to get board_type and the constexpr variables that depend on it).
class Square;

class Board
{
public:
  static constexpr unsigned int board_size = 16;
  // The number of bits needed to store a single coordinate (horizontal or vertical: the board is square).
  static constexpr int coord_bits = utils::ceil_log2(board_size);
  // A square has two coordinates: x and y.
  static constexpr int square_bits = 2 * coord_bits;
  // A board has three pieces: black king, white king and white rook.
  static constexpr int total_required_bits = 3 * square_bits;
  // The size of Graph::black_to_move_ and Graph::white_to_move_.
  static constexpr size_t color_to_move_size = size_t{1} << total_required_bits;
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

  static constexpr coordinates_type white_rook_mask = square_mask;
  static constexpr coordinates_type white_king_mask = white_rook_mask << white_king_shift;
  static constexpr coordinates_type black_king_mask = white_rook_mask << black_king_shift;

  static constexpr SquareCompact::coordinates_type xy_to_coordinates(int x, int y)
  {
    // A SquareCompact::coordinates_type contains <y-coord-bits><x-coord-bits>.
    SquareCompact::coordinates_type square_coordinates = x;
    square_coordinates |= y << coord_bits;
    return square_coordinates;
  }

  // Extract the x-coordinate from a SquareCompact.
  static constexpr int x_coord(SquareCompact square)
  {
    return square.coordinates() & coord_mask;
  }

  // Extract the y-coordinate from a SquareCompact.
  static constexpr int y_coord(SquareCompact square)
  {
    return (square.coordinates() >> coord_bits) & coord_mask;
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
  Board(size_t index) : coordinates_(index) { ASSERT(index < color_to_move_size); }
  Board(SquareCompact black_king, SquareCompact white_king, SquareCompact white_rook) :
    coordinates_(black_king.coordinates() << black_king_shift |
                 white_king.coordinates() << white_king_shift |
                 white_rook.coordinates())
  {
    Debug(black_king.sane_coordinates(board_size));
    Debug(white_king.sane_coordinates(board_size));
    Debug(white_rook.sane_coordinates(board_size));
  }

  // Accessors.
  // Convert Board piece to a SquareCompact.
  SquareCompact black_king() const { return (coordinates_ >> black_king_shift) & square_mask; }
  SquareCompact white_king() const { return (coordinates_ >> white_king_shift) & square_mask; }
  SquareCompact white_rook() const { return coordinates_ & square_mask; }
  size_t as_index() const { return coordinates_; }

  bool distance_less(Board const& board) const
  {
    return coordinates_ < board.coordinates_;
  }

  bool operator==(Board const& other) const {
    return coordinates_ == other.coordinates_;
  };

  std::string get_move(Board const& to_board);

  enum class Figure {
    none,
    black_king,
    white_king,
    white_rook
  };

  static void utf8art(std::ostream& os, Color to_move, bool xyz, std::function<Figure (Square)> select_figure);
  void utf8art(std::ostream& os, Color to_move, bool xyz = false) const;

  void set_black_king_square(SquareCompact black_king)
  {
    Debug(black_king.sane_coordinates(board_size));
    coordinates_ &= ~black_king_mask;
    coordinates_ |= black_king.coordinates() << black_king_shift;
  }

  void set_white_king_square(SquareCompact white_king)
  {
    Debug(white_king.sane_coordinates(board_size));
    coordinates_ &= ~white_king_mask;
    coordinates_ |= white_king.coordinates() << white_king_shift;
  }

  void set_white_rook_square(SquareCompact white_rook)
  {
    Debug(white_rook.sane_coordinates(board_size));
    coordinates_ &= ~white_rook_mask;
    coordinates_ |= white_rook.coordinates();
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
    SquareCompact bk = black_king();
    SquareCompact wk = white_king();
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
  inline std::tuple<Square, Square, Square> abbreviations() const;

#if CW_DEBUG
 private:
  // By default, assume black is to move (only used for an assert).
  virtual bool black_is_to_move() const { return true; }
#endif
};

//inline
constexpr SquareCompact::SquareCompact(int x, int y) : coordinates_(Board::xy_to_coordinates(x, y))
{
}

#ifdef CWDEBUG
inline void SquareCompact::sane_coordinates(int board_size) const
{
  int x = Board::x_coord(*this);
  int y = Board::y_coord(*this);
  ASSERT(0 <= x && x < board_size);
  ASSERT(0 <= y && y < board_size);
}
#endif
