#pragma once

#include "Color.h"
#include "utils/has_print_on.h"
#include <cstdint>
#include "debug.h"

#ifdef CWDEBUG
// This class defines a print_on method.
using utils::has_print_on::operator<<;
#endif

namespace version0 {

class Board;

class Classification
{
 public:
  static constexpr uint8_t mate = 1;
  static constexpr uint8_t stalemate = 2;
  static constexpr uint8_t draw = 4;
  static constexpr uint8_t check = 8;

 protected:
  std::uint8_t bits_{};
  int mate_in_moves_{-1};       // Number of moves (in ply) required to reach mate under optimal play, or -1 if unknown.

 public:
  void determine(Board const& board, Color to_move);

 private:
  // Clear all classification bits.
  void reset()
  {
    bits_ = 0;
  }
  // Set the check bit.
  void set_check() { bits_ |= check; }
  // Set the draw bit.
  void set_draw() { bits_ |= draw; }
  // Set the mate bit.
  void set_mate() { bits_ |= mate; }
  // Set the stalemate bit.
  void set_stalemate() { bits_ |= stalemate; }
 public:
  // Set in how many ply this position is mate.
  void set_mate_in_ply(int ply)
  {
    ASSERT(ply >= 0);
    // If it is a draw, then it isn't mate in `ply` moves; so why is this function being called?
    ASSERT(!is_draw());
    // If it mate then `ply` must be zero.
    ASSERT(is_mate() == (ply == 0));
    mate_in_moves_ = ply;
  }

 public:
  // Accessors.
  bool has_classification(uint8_t classification_mask) const { return (classification_mask & bits_) == classification_mask; }
  bool is_mate() const { return (bits_ & mate); }
  bool is_stalemate() const { return (bits_ & stalemate); }
  bool is_draw() const { return (bits_ & draw); }
  bool is_check() const { return (bits_ & check); }
  int ply() const { return mate_in_moves_; }

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};

} // namespace version0

namespace version1 {

class Board;

class Classification
{
 public:
  static constexpr uint8_t mate = 1;
  static constexpr uint8_t stalemate = 2;
  static constexpr uint8_t draw = 4;
  static constexpr uint8_t check = 8;
  static constexpr uint8_t legal = 16;

 protected:
  std::uint8_t bits_{};
  int mate_in_moves_{-1};       // Number of moves (in ply) required to reach mate under optimal play, or -1 if unknown.

 public:
  void determine(Board const& board, Color to_move);

 private:
  // Clear all classification bits.
  void reset()
  {
    bits_ = 0;
  }
  // Set the check bit.
  void set_check() { bits_ |= check; }
  // Set the draw bit.
  void set_draw() { bits_ |= draw; }
  // Set the mate bit.
  void set_mate() { bits_ |= mate; }
  // Set the stalemate bit.
  void set_stalemate() { bits_ |= stalemate; }
  // Set the legal bit to mark the other bits as valid.
  // Only invalid positions remain having this bit unset.
  void set_legal() { bits_ |= legal; }
 public:
  // Set in how many ply this position is mate.
  void set_mate_in_ply(int ply)
  {
    ASSERT(ply >= 0);
    // If it is a draw, then it isn't mate in `ply` moves; so why is this function being called?
    ASSERT(!is_draw());
    // If it mate then `ply` must be zero.
    ASSERT(is_mate() == (ply == 0));
    mate_in_moves_ = ply;
  }

 public:
  // Accessors.
  bool has_classification(uint8_t classification_mask) const { return (classification_mask & bits_) == classification_mask; }
  bool is_mate() const { return (bits_ & mate); }
  bool is_stalemate() const { return (bits_ & stalemate); }
  bool is_draw() const { return (bits_ & draw); }
  bool is_check() const { return (bits_ & check); }
  bool is_legal() const { return (bits_ & legal); }
  int ply() const { return mate_in_moves_; }

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};

} // namespace version1
