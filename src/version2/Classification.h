#pragma once

#include "Color.h"
#include "Size.h"
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
  // Storing a ply.
  // The given formula is exact for square boards of NxN where 12 <= N <= 32.
  static constexpr unsigned int max_ply_upperbound = 2 * ((33 * std::max(Size::board_size_x, Size::board_size_y) - 34) / 7);
  // We need to add two extra because
  //    I) unknown_ply may not used, and
  //    II) set_maximum_ply_on_parents tests against the last found ply (that can have
  //    the maximum value returned by this formula) PLUS one and that still needs to be less than unknown_ply.
  // For example, for board_size = 28 the maximum possible number of ply that a position is mate is 254,
  // which means we need to be able to have 255 (one more) available as less than unknown_ply: unknown_ply >= 256 -->
  // unknown_ply = 511 and we need 9 bits.
  static constexpr int ply_bits = utils::log2(max_ply_upperbound + 2) + 1;

  static constexpr int number_of_bits = 5;
  static constexpr int encoded_bits = ply_bits + number_of_bits;
  using encoded_type = uint_type<encoded_bits>;

  static constexpr int mate_in_ply_shift = number_of_bits;
  static constexpr encoded_type mate_in_ply_mask = create_mask<encoded_type, ply_bits>() << mate_in_ply_shift;

  using ply_type = uint_type<ply_bits>;
  static constexpr ply_type unknown_ply = create_mask<encoded_type, ply_bits>();

  static constexpr encoded_type mate = 1;
  static constexpr encoded_type stalemate = 2;
  static constexpr encoded_type draw = 4;
  static constexpr encoded_type check = 8;
  static constexpr encoded_type legal = 16;
  static constexpr encoded_type bits_mask = create_mask<encoded_type, number_of_bits>();

 private:
  encoded_type encoded_;                        // <mate_in_moves><bits>

 public:
  // Initialize <mate_in_ply> with unknown_ply.
  Classification() : encoded_{static_cast<encoded_type>(unknown_ply) << mate_in_ply_shift} { }

  void determine(Board const& board, Color to_move);

 private:
  // Clear all classification bits.
  void reset()
  {
    encoded_ &= ~bits_mask;
  }
  // Set the check bit.
  void set_check() { encoded_ |= check; }
  // Set the draw bit.
  void set_draw() { encoded_ |= draw; }
  // Set the mate bit.
  void set_mate() { encoded_ |= mate; }
  // Set the stalemate bit.
  void set_stalemate() { encoded_ |= stalemate; }
  // Set the legal bit to mark the other bits as valid.
  // Only invalid positions remain having this bit unset.
  void set_legal() { encoded_ |= legal; }

 public:
  // Set in how many ply this position is mate.
  void set_mate_in_ply(ply_type ply)
  {
    // Only call this function with an argument that makes sense.
    ASSERT(ply < unknown_ply);
    // If it is a draw, then it isn't mate in `ply` moves; so why is this function being called?
    ASSERT(!is_draw());
    // If it mate then `ply` must be zero.
    ASSERT(is_mate() == (ply == 0));
    encoded_ &= ~mate_in_ply_mask;
    encoded_ |= static_cast<encoded_type>(ply) << mate_in_ply_shift;
  }

  // Accessors.
  bool is_mate() const { return (encoded_ & mate); }
  bool is_stalemate() const { return (encoded_ & stalemate); }
  bool is_draw() const { return (encoded_ & draw); }
  bool is_check() const { return (encoded_ & check); }
  bool is_legal() const { return (encoded_ & legal); }
  int ply() const { return encoded_ >> mate_in_ply_shift; }

  // Serialization.
  void write_to(std::ostream& os) const;
  void read_from(std::istream& is);

  friend bool operator==(Classification lhs, Classification rhs)
  {
    return lhs.encoded_ == rhs.encoded_;
  }

#ifdef CWDEBUG
  static std::string state_str(encoded_type state);
  void print_on(std::ostream& os) const;
#endif
};
