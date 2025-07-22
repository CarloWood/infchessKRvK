#pragma once

#include "Board.h"
#include "Color.h"
#include "utils/has_print_on.h"
#include <cstdint>
#include <vector>
#include <array>
#include <iostream>
#include "debug.h"

#ifdef CWDEBUG
// This class defines a print_on method.
using utils::has_print_on::operator<<;
#endif

class Position : public Board
{
public:
  enum class Mate : std::uint8_t
  {
    no,
    stalemate,
    yes
  };

  struct Classification
  {
    static constexpr uint8_t mate = 1;
    static constexpr uint8_t stalemate = 2;
    static constexpr uint8_t draw = 4;
    static constexpr uint8_t check = 8;
  };

private:
  Color to_move_;
  std::uint8_t classification_;

 public:
  Position(int board_size, Square const& black_king, Square const& white_king, Square const& white_rook, Color to_move) :
    Board(board_size, black_king, white_king, white_rook), to_move_(to_move), classification_{} { }

 public:
  // Accessors.
  Color to_move() const { return to_move_; }
  bool has_classification(uint8_t classification_mask) const { return (classification_mask & classification_) == classification_mask; }
  bool is_mate() const { return (classification_ & Classification::mate); }
  bool is_stalemate() const { return (classification_ & Classification::stalemate); }
  bool is_draw() const { return (classification_ & Classification::draw); }
  bool is_check() const { return (classification_ & Classification::check); }

  // Returns all possible legal positions, already classified.
  static std::vector<Position> analyze_all(int board_size);

 private:
  // These are only called for legal positions:
  Mate determine_mate() const;          // Uses black_has_moves and determine_check.
  bool determine_draw() const;          // Uses determine_mate.

  void classify();

  // Toggle whose turn it is.
  void null_move()
  {
    to_move_ = to_move_.opponent();
  }

  // Convert to FEN code (only works if horizontal_limit = vertical_limit = 8).
  std::string to_fen() const;

#if CW_DEBUG
  bool black_is_to_move() const override
  {
    return to_move_ == black;
  }
#endif

 public:
#ifdef CWDEBUG
  void print_on(std::ostream& os) const;

  // Print the board as utf8art with Dout.
  void debug_utf8art(libcwd::channel_ct const& debug_channel) const
  {
    Board::debug_utf8art(debug_channel);
    if (board_size_ == 8)
      Dout(debug_channel, to_fen());
  }
#endif
};

std::ostream& operator<<(std::ostream& os, Position::Mate mate);

inline std::ostream& operator<<(std::ostream& os, Position const& pos)
{
  pos.print_on(os);
  return os;
}

#if 0
template<typename T, size_t number_of_implementations>
T Position::determine_results(std::array<T (Position::*)() const, number_of_implementations> const& member_function_array,
    char const* fnname) const
{
  std::array<T, number_of_implementations> results;
  for (int i = 0; i < number_of_implementations; ++i)
    results[i] = (this->*member_function_array[i])();
  T result = results[0];      // The first implementation is assumed correct.
  for (int i = 1; i < number_of_implementations; ++i)
  {
    if (results[i] != result)
    {
      std::cerr << "Disagreement on " << fnname << " for " << *this << std::endl;
      utf8art(std::cerr);
      std::cerr << fnname << "1 = " << result << ", " << fnname << (i + 1) << " = " << std::boolalpha << results[i] << "." << std::endl;
    }
  }
  return result;
}
#endif
