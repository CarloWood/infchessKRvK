#pragma once

#include "Board.h"
#include "Info.h"
#include "utils/Vector.h"

class Graph
{
 public:
  using info_nodes_type = Info::nodes_type;
  static constexpr size_t info_nodes_size = size_t{1} << Board::encoded_bits;

 private:
  info_nodes_type black_to_move_{info_nodes_size};
  info_nodes_type white_to_move_{info_nodes_size};

 public:
  Info::nodes_type const& black_to_move() const { return black_to_move_; }
  Info::nodes_type& black_to_move() { return black_to_move_; }
  Info::nodes_type const& white_to_move() const { return white_to_move_; }
  Info::nodes_type& white_to_move() { return white_to_move_; }

  void classify();

  void write_to(std::ostream& os) const;
  void read_from(std::istream& is);
};
