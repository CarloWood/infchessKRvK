#pragma once

#include "BlackKing.h"
#include "WhiteKing.h"
#include "Rook.h"
#include "utils/has_print_on.h"
#include <vector>
#include <iostream>
#include "debug.h"

#ifdef CWDEBUG
// This class defines a print_on method.
using utils::has_print_on::operator<<;
#endif

class Board
{
 public:
  static constexpr int vertical_limit = 16;
  static constexpr int vertical_limit_printing = vertical_limit;
  static constexpr int horizontal_limit = vertical_limit;
  static constexpr int horizontal_limit_printing = horizontal_limit;

 private:
  BlackKing bK_;
  WhiteKing wK_;
  Rook wR_;
  Color to_play_;

  bool print_flipped_{false};   // True if the position mirrored in the n = m line should be shown when printing this position.

 public:
  // Default constructor: the first legal position:
  //   ┏━0━1━2━...
  // 0 ┃ ♔ ♜ ♚
  Board() : bK_({0, 0}, black), wK_({2, 0}, white), wR_({1, 0}), to_play_{black} { }

  // Construct a board for a given position of the pieces.
  Board(Square bk, Square wk, Square wr, Color to_play, bool mirror);

  // Accessors.
  BlackKing const& bK() const { return bK_; }
  WhiteKing const& wK() const { return wK_; }
  Rook const& wR() const { return wR_; }
  Color to_play() const { return to_play_; }
  bool print_flipped() const { return print_flipped_; }

  bool distance_less(Board const& board) const;
  bool is_illegal() const;
  bool is_canonical() const;

  // Show the board.
  void print_to(std::ostream& os) const;

  // Convert to FEN code (only works if horizontal_limit = vertical_limit = 8).
  std::string to_fen() const;

  // Mirror the whole board in the n = m line.
  void mirror();

  // Toggle whose turn it is.
  void null_move()
  {
    to_play_ = to_play_.next();
  }

  void set_black_king_square(Square bk_pos)
  {
    bK_.pos() = bk_pos;
  }

  void set_white_king_square(Square wk_pos)
  {
    wK_.pos() = wk_pos;
  }

  void set_white_rook_square(Square wr_pos)
  {
    wR_.pos() = wr_pos;
  }

  // Show the board using UTF8 art.
  void utf8art(std::ostream& os) const;

#ifdef CWDEBUG
  // Allow printing a Board to an ostream.
  void print_on(std::ostream& os) const;

  // Print the board as utf8art with Dout.
  void debug_utf8art(libcwd::channel_ct const& debug_channel) const;
#endif

 private:
  void canonicalize(bool mirror);
};

struct DistanceCompare
{
  bool operator()(Board const& lhs, Board const& rhs) const
  {
    return lhs.distance_less(rhs);
  }
};
