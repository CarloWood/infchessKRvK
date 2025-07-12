#pragma once

#include "BlackKing.h"
#include "WhiteKing.h"
#include "Rook.h"
#include <vector>
#include <iostream>

class Board
{
 public:
  static constexpr int vertical_limit = 8;
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
  Board(Square bk, Square wk, Square wr, Color to_play) : bK_(bk, black), wK_(wk, white), wR_(wr), to_play_(to_play)
  {
    std::cout << "Constructing Board(" << bk << ", " << wk << ", " << wr << ", " << to_play << "); board is now: " << *this << " [" << this << "]\n";
    canonicalize();
  }

  // Accessors.
  BlackKing const& bK() const { return bK_; }
  WhiteKing const& wK() const { return wK_; }
  Rook const& wR() const { return wR_; }
  Color to_play() const { return to_play_; }

  bool distance_less(Board const& board) const;
  bool is_illegal() const;
  bool is_canonical() const { return bK_.is_canonical(); }

  // Show the board.
  void print() const;

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

  // Generate all positions that can reach this position in one ply.
  std::vector<Board> preceding_positions() const;

  // Allow printing a Board to an ostream.
  friend std::ostream& operator<<(std::ostream& os, Board const& board);

 private:
  void canonicalize();
};
