#pragma once

#include "Partition.h"
#include "PartitionElement.h"
#include "Info.h"
#include "Board.h"
#include "utils/Array.h"

class Graph
{
 public:
  static constexpr size_t partitions_size = Size::Px * Size::Py;
  using partitions_type = utils::Array<Info::nodes_type, partitions_size, PartitionIndex>;

 private:
  partitions_type black_to_move_;
  partitions_type white_to_move_;

 public:
  void classify();

  template<color_type to_move>
  Info& get_info(Board board)
  {
    partitions_type& color_to_move = to_move == black ? black_to_move_ : white_to_move_;
    return color_to_move[board.as_partition()][board.as_partition_element()];
  }

  template<color_type to_move>
  Info const& get_info(Board board) const
  {
    partitions_type const& color_to_move = to_move == black ? black_to_move_ : white_to_move_;
    return color_to_move[board.as_partition()][board.as_partition_element()];
  }

  template<color_type to_move>
  Info& get_info(Partition partition, PartitionElement partition_element)
  {
    partitions_type& color_to_move = to_move == black ? black_to_move_ : white_to_move_;
    return color_to_move[partition][partition_element];
  }

  partitions_type const& black_to_move_partition() const
  {
    return black_to_move_;
  }

  partitions_type const& white_to_move_partition() const
  {
    return white_to_move_;
  }

  void write_to(std::ostream& os) const;
  void read_from(std::istream& is);
};
