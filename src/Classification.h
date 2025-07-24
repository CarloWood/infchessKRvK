#pragma once

#include "utils/has_print_on.h"
#include <cstdint>

#ifdef CWDEBUG
// This class defines a print_on method.
using utils::has_print_on::operator<<;
#endif

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
  // Set in how many ply this position is mate.
  void set_mate_in_ply(int ply) { mate_in_moves_ = ply; }

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
