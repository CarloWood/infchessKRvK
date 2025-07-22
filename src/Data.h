#pragma once

#include "Board.h"
#include <vector>
#include <map>

class Data
{
 public:
  // Must be the same type as Graph::nodes_type.
  using nodes_type = std::map<Board, Data, DistanceCompare>;

  struct Classification
  {
    static constexpr uint8_t mate = 1;
    static constexpr uint8_t stalemate = 2;
    static constexpr uint8_t draw = 4;
    static constexpr uint8_t check = 8;
  };

 private:
  std::uint8_t classification_;
  int mate_in_moves_;                   // Number of moves (in ply) required to reach mate under optimal play.
  std::vector<nodes_type::const_iterator> possible_moves_;

 public:
  Data(int ply) : mate_in_moves_(ply) { }

  // Accessors.
  bool has_classification(uint8_t classification_mask) const { return (classification_mask & classification_) == classification_mask; }
  bool is_mate() const { return (classification_ & Classification::mate); }
  bool is_stalemate() const { return (classification_ & Classification::stalemate); }
  bool is_draw() const { return (classification_ & Classification::draw); }
  bool is_check() const { return (classification_ & Classification::check); }
  int ply() const { return mate_in_moves_; }
  std::vector<nodes_type::const_iterator> const& possible_moves() const { return possible_moves_; }

  void add_edge(nodes_type::const_iterator const& next_board);
};
