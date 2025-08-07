#pragma once

#include "Color.h"
#include "Square.h"
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
  static constexpr int black_king_shift = WhiteRookSquare::bits + WhiteKingSquare::bits;
  static constexpr int white_king_shift = WhiteRookSquare::bits;
  static constexpr encoded_type black_king_mask = BlackKingSquare::mask;
  static constexpr encoded_type white_king_mask = WhiteKingSquare::mask;
  static constexpr encoded_type white_rook_mask = WhiteRookSquare::mask;

  // Storing a ply.
  static constexpr unsigned int max_ply_estimate = 256;
  static constexpr int ply_bits = utils::ceil_log2(max_ply_estimate);
  using ply_type = uint_type<ply_bits>;

  // Number of child positions.
  static constexpr unsigned int max_number_of_children =
    Size::board::x + Size::board::y + 6;        // Number of rook moves plus number of king moves, including illegal ones.
  static constexpr int number_of_children_bits = utils::ceil_log2(max_number_of_children);
  using number_of_children_type = uint_type<number_of_children_bits>;

 private:
  encoded_type encoded_;        // Compact representation of <BlackKingSquare><WhiteKingSquare><WhiteRookSquare>.

 public:
  Board(BlackKingSquare const& bk, WhiteKingSquare const& wk, WhiteRookSquare const& wr) :
    encoded_((encoded_type{bk.coordinates()} << black_king_shift) |
             (encoded_type{wk.coordinates()} << white_king_shift) |
              encoded_type{wr.coordinates()}) { }

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
    encoded_type bk_square;
    bk_square = encoded_ >> black_king_shift;
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

 private:
  // Used by black_has_moves, determine_check, determine_draw and get_succeeding_boards.
  std::tuple<Square, Square, Square> abbreviations() const;
};
