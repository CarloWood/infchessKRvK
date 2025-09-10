#pragma once

#include "Size.h"
#include "uint_type.h"
#include "../Color.h"
#include "utils/has_print_on.h"
#include <cstdint>
#include <string>
#include <atomic>
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
  //    the maximum value returned by this formula) PLUS one and that still needs to be less than std::numeric_limits<ply_type>::max().
  // For example, for board_size = 28 the maximum possible number of ply that a position is mate is 254,
  // which means we need to be able to have 255 (one more) available as less than 'power of 2 minus 1' (the max()): max >= 256 -->
  // max = 511 and we need 9 bits.
  static constexpr int ply_bits = utils::log2(max_ply_upperbound + 2) + 1;

  static constexpr int number_of_bits = 5;
  static constexpr int encoded_bits = ply_bits + number_of_bits;
  using encoded_type = uint_type<encoded_bits>;

  static constexpr int mate_in_ply_shift = number_of_bits;
  static constexpr encoded_type max_encoded_ply = create_mask<encoded_type, ply_bits>();
  static constexpr encoded_type mate_in_ply_mask = max_encoded_ply << mate_in_ply_shift;


  using ply_type = uint_type<ply_bits>;
  static constexpr ply_type encoded_unknown_ply = 0;
  static constexpr int unknown_ply = -1;

  static constexpr encoded_type mate = 1;
  static constexpr encoded_type stalemate = 2;
  static constexpr encoded_type draw = 4;
  static constexpr encoded_type check = 8;
  static constexpr encoded_type legal = 16;
  static constexpr encoded_type bits_mask = create_mask<encoded_type, number_of_bits>();

 private:
  // mmap-ed data can not be atomic.
  encoded_type encoded_;                        // <mate_in_moves><bits>

 public:
  // Do nothing if this is the default constructor;
  // in that case we make use of the fact that this is constructed with
  // placement-new on a memory image that already contains initialized data.
  Classification(bool initialize = false) { if (initialize) Classification::initialize(); }

  // Call this if the underlying memory was not already initialized.
  void initialize()
  {
    encoded_ = 0;       // Not mate, stalemate, draw, check or legal(!) - and mate_in_ply is unknown.
  }

  // Call this if the underlying memory was already initialized, but we want to calculate the number of ply again (for testing).
  void reset_ply()
  {
    encoded_ &= bits_mask;
  }

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
    // If it is a draw, then it isn't mate in `ply` moves; so why is this function being called?
    ASSERT(!is_draw());
    // If it mate then `ply` must be zero.
    ASSERT(is_mate() == (ply == 0));

    // Add +1 to the encoded ply because zero means "unknown".
    encoded_ = (encoded_ & ~mate_in_ply_mask) | (static_cast<encoded_type>(ply + 1) << mate_in_ply_shift);
  }

  // Set in how many ply this position is mate.
  [[nodiscard]] bool set_mate_in_ply(std::atomic<uint8_t>& winner, ply_type ply)
  {
    // The bits corresponding to mate_in_ply_mask are initially all zero (meaning "unknown"; aka encoded_unknown_ply).
    // Those bits are changed once, by one thread, to something non-zero (iff the corresponding position is legal and not a is_draw),
    // and never changed again.
    // The bits are only changed by this function.
    // There is a race condition between threads, where multiple threads can call this function concurrently.
    // In that case it doesn't matter which thread makes the change, because it is guaranteed that all pass the same value for `ply`.
    // However, only one of them is allowed to return true.

    // Only the thread that increments `winner` first is allowed to return true (and make the change).
    if (winner.fetch_add(1, std::memory_order_relaxed) != 0)
      return false;

    set_mate_in_ply(ply);
    return true;
  }

  // Accessors.
  bool is_mate() const { return (encoded_ & mate); }
  bool is_stalemate() const { return (encoded_ & stalemate); }
  bool is_draw() const { return (encoded_ & draw); }
  bool is_check() const { return (encoded_ & check); }
  bool is_legal() const { return (encoded_ & legal); }
  // Subtract 1 again to undo the +1 in set_mate_in_ply.
  int ply() const { return (encoded_ >> mate_in_ply_shift) - 1; }

#if CW_DEBUG
  // This is only used in single threaded code (for debugging purposes).
  friend bool operator==(Classification const& lhs, Classification const& rhs)
  {
    return lhs.encoded_ == rhs.encoded_;
  }
#endif

#ifdef CWDEBUG
  static std::string state_str(encoded_type state);
  void print_on(std::ostream& os) const;
#endif
};
