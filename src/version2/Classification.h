#pragma once

#include "Color.h"
#include "uint_type.h"
#include "utils/has_print_on.h"
#include <cstdint>
#include <string>
#include "debug.h"

#ifdef CWDEBUG
// This class defines a print_on method.
using utils::has_print_on::operator<<;
#endif

class Board;

class Classification
{
 public:
  static constexpr int number_of_bits = 5;
  using bits_type = uint_type<number_of_bits>;

  static constexpr bits_type mate = 1;
  static constexpr bits_type stalemate = 2;
  static constexpr bits_type draw = 4;
  static constexpr bits_type check = 8;
  static constexpr bits_type legal = 16;
  static constexpr bits_type mask = ~bits_type{0} >> (8 - number_of_bits);

 protected:
  bits_type bits_{};

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
  // Accessors.
  bool has_classification(bits_type classification_mask) const { return (classification_mask & bits_) == classification_mask; }
  bool is_mate() const { return (bits_ & mate); }
  bool is_stalemate() const { return (bits_ & stalemate); }
  bool is_draw() const { return (bits_ & draw); }
  bool is_check() const { return (bits_ & check); }
  bool is_legal() const { return (bits_ & legal); }

#ifdef CWDEBUG
  static std::string state_str(bits_type state);
  void print_on(std::ostream& os) const;
#endif
};
