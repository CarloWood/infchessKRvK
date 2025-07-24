#pragma once

#include "Square.h"
#include "Color.h"
#include "utils/has_print_on.h"
#include <functional>
#include <tuple>
#include "debug.h"

#ifdef CWDEBUG
// This class defines a print_on method.
using utils::has_print_on::operator<<;
#endif

class Board
{
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

  bool distance_less(Board const& board) const;

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

 protected:
  bool black_has_moves() const;
  bool determine_check() const;

 public:
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
