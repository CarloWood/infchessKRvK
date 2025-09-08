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

class Graph
{
 public:
  static constexpr size_t number_of_partitions = utils::square(Size::Px * Size::Py);
  using partitions_type = utils::Array<Info::nodes_type, number_of_partitions, PartitionIndex>;

 private:
  std::filesystem::path data_directory_;
  memory::MemoryMappedPool partitions_pool_;
  bool reuse_file_;
  std::unique_ptr<partitions_type, std::function<void(partitions_type*)>> black_to_move_;
  std::unique_ptr<partitions_type, std::function<void(partitions_type*)>> white_to_move_;

  size_t partitions_size()
  {
    size_t const memory_page_size = memory::MemoryMappedPool::memory_page_size();
    return utils::nearest_multiple_of_power_of_two(sizeof(partitions_type), memory_page_size);
  }

  void* black_to_move_partitions_start()
  {
    if (reuse_file_)
      return partitions_pool_.mapped_base();
    else
      return partitions_pool_.allocate();
  }

  void* white_to_move_partitions_start()
  {
    if (reuse_file_)
      return static_cast<char*>(partitions_pool_.mapped_base()) + partitions_size();
    else
      return partitions_pool_.allocate();
  }

 public:
  Graph(std::filesystem::path prefix_directory, bool reuse_file) :
    data_directory_(data_directory(prefix_directory)),
    partitions_pool_(data_filename(prefix_directory), partitions_size(), 2 * partitions_size(),
        memory::MemoryMappedPool::Mode::persistent, !reuse_file),
    reuse_file_(reuse_file),
    black_to_move_(new (black_to_move_partitions_start()) partitions_type, [this](partitions_type* ptr){ }),
    white_to_move_(new (white_to_move_partitions_start()) partitions_type, [this](partitions_type* ptr){ })
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

  void reset_ply()
  {
    // This sets everything to zero.
    for (Info::nodes_type& nodes : *black_to_move_)
      for (Info& info : nodes)
        info.reset_ply();
    for (Info::nodes_type& nodes : *white_to_move_)
      for (Info& info : nodes)
        info.reset_ply();
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

  static std::filesystem::path data_directory(std::filesystem::path const& prefix_directory);
  static std::filesystem::path data_filename(std::filesystem::path const& prefix_directory)
  {
    return Graph::data_directory(prefix_directory) / "mmap.img";
  }
};
