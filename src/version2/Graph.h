#pragma once

#include "Partition.h"
#include "PartitionElement.h"
#include "Info.h"
#include "Board.h"
#include "memory/MemoryMappedPool.h"
#include "utils/Array.h"
#include "utils/nearest_multiple_of_power_of_two.h"
#include "utils/square.h"
#include <filesystem>

// Allow using `new (pool) Foo`.
inline void* operator new(std::size_t size, memory::MemoryMappedPool& pool) { return pool.allocate(); }

class Graph
{
 public:
  static constexpr size_t number_of_partitions = utils::square(Size::Px * Size::Py);
  using partitions_type = utils::Array<Info::nodes_type, number_of_partitions, PartitionIndex>;

 private:
  std::filesystem::path partition_directory_;
  memory::MemoryMappedPool partitions_pool_;
  std::unique_ptr<partitions_type, std::function<void(partitions_type*)>> black_to_move_;
  std::unique_ptr<partitions_type, std::function<void(partitions_type*)>> white_to_move_;

  size_t partitions_size()
  {
    size_t const memory_page_size = memory::MemoryMappedPool::memory_page_size();
    return utils::nearest_multiple_of_power_of_two(sizeof(partitions_type), memory_page_size);
  }

 public:
  Graph(std::filesystem::path directory, bool zero_init) :
    partition_directory_(
      directory /
      std::format("board{}x{}", Size::board_size_x, Size::board_size_y) /
      std::format("partition{}x{}", Size::Px, Size::Py)),
    partitions_pool_(partition_directory_ / "mmap.img", partitions_size(), 2 * partitions_size(),
        memory::MemoryMappedPool::Mode::persistent, zero_init),
    black_to_move_(new (partitions_pool_) partitions_type, [this](partitions_type* ptr){ partitions_pool_.deallocate(ptr); }),
    white_to_move_(new (partitions_pool_) partitions_type, [this](partitions_type* ptr){ partitions_pool_.deallocate(ptr); })
    { }

  void initialize()
  {
    // This sets everything to zero.
    for (Info::nodes_type& nodes : *black_to_move_)
      for (Info& info : nodes)
        info.initialize();
    for (Info::nodes_type& nodes : *white_to_move_)
      for (Info& info : nodes)
        info.initialize();
  }

  void classify();

  template<color_type to_move>
  Info& get_info(Board board)
  {
    partitions_type& color_to_move = to_move == black ? *black_to_move_ : *white_to_move_;
    return color_to_move[board.as_partition()][board.as_partition_element()];
  }

  template<color_type to_move>
  Info const& get_info(Board board) const
  {
    partitions_type const& color_to_move = to_move == black ? *black_to_move_ : *white_to_move_;
    return color_to_move[board.as_partition()][board.as_partition_element()];
  }

  template<color_type to_move>
  Info& get_info(Partition partition, PartitionElement partition_element)
  {
    partitions_type& color_to_move = to_move == black ? *black_to_move_ : *white_to_move_;
    return color_to_move[partition][partition_element];
  }

  template<color_type to_move>
  Info const& get_info(Partition partition, PartitionElement partition_element) const
  {
    partitions_type& color_to_move = to_move == black ? *black_to_move_ : *white_to_move_;
    return color_to_move[partition][partition_element];
  }

  partitions_type const& black_to_move_partition() const
  {
    return *black_to_move_;
  }

  partitions_type const& white_to_move_partition() const
  {
    return *white_to_move_;
  }

  void write_to(std::ostream& os) const;
  void read_from(std::istream& is);

  std::filesystem::path partition_filename(Partition partition) const;
};
