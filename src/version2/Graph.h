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
#include <tuple>

class Graph
{
 public:
  static constexpr size_t number_of_partitions = utils::square(Size::Px * Size::Py);
  using infos_type = utils::Array<Info::nodes_type, number_of_partitions, PartitionIndex>;
  using non_mapped_infos_type = utils::Array<NonMappedInfo::nodes_type, number_of_partitions, PartitionIndex>;

 private:
  std::filesystem::path data_directory_;
  memory::MemoryMappedPool partitions_pool_;
  bool reuse_file_;
  std::unique_ptr<infos_type, std::function<void(infos_type*)>> black_to_move_infos_;
  std::unique_ptr<non_mapped_infos_type> black_to_move_non_mapped_infos_;
  std::unique_ptr<infos_type, std::function<void(infos_type*)>> white_to_move_infos_;
  std::unique_ptr<non_mapped_infos_type> white_to_move_non_mapped_infos_;

  // The space allocated for an `infos_type` array.
  // This is also the offset (in the memory mapped file) between the black_to_move_infos_ and the white_to_move_infos_ array.
  size_t infos_size()
  {
    size_t const memory_page_size = memory::MemoryMappedPool::memory_page_size();
    return utils::nearest_multiple_of_power_of_two(sizeof(infos_type), memory_page_size);
  }

  void* black_to_move_infos_start()
  {
    if (reuse_file_)
      return partitions_pool_.mapped_base();
    else
      return partitions_pool_.allocate();
  }

  void* white_to_move_infos_start()
  {
    if (reuse_file_)
      return static_cast<char*>(partitions_pool_.mapped_base()) + infos_size();
    else
      return partitions_pool_.allocate();
  }

 public:
  Graph(std::filesystem::path prefix_directory, bool reuse_file, bool read_only = false) :
    data_directory_(data_directory(prefix_directory)),
    partitions_pool_(data_filename(prefix_directory), infos_size(), 2 * infos_size(),
        read_only ? memory::MemoryMappedPool::Mode::copy_on_write : memory::MemoryMappedPool::Mode::persistent, !reuse_file),
    reuse_file_(reuse_file),
    black_to_move_infos_(new (black_to_move_infos_start()) infos_type, [this](infos_type* ptr){ }),
    black_to_move_non_mapped_infos_(std::make_unique<non_mapped_infos_type>()),
    white_to_move_infos_(new (white_to_move_infos_start()) infos_type, [this](infos_type* ptr){ }),
    white_to_move_non_mapped_infos_(std::make_unique<non_mapped_infos_type>())
    { }

  void initialize()
  {
    // This sets everything to zero.
    for (Info::nodes_type& nodes : *black_to_move_infos_)
      for (Info& info : nodes)
        info.initialize();
    for (NonMappedInfo::nodes_type& nodes : *black_to_move_non_mapped_infos_)
      for (NonMappedInfo& non_mapped_info : nodes)
        non_mapped_info.initialize();
    for (Info::nodes_type& nodes : *white_to_move_infos_)
      for (Info& info : nodes)
        info.initialize();
    for (NonMappedInfo::nodes_type& nodes : *white_to_move_non_mapped_infos_)
      for (NonMappedInfo& non_mapped_info : nodes)
        non_mapped_info.initialize();
  }

  void reset_ply()
  {
    // This sets everything required to recalculate the ply to zero.
    for (Info::nodes_type& nodes : *black_to_move_infos_)
      for (Info& info : nodes)
        info.reset_ply();
    for (NonMappedInfo::nodes_type& nodes : *black_to_move_non_mapped_infos_)
      for (NonMappedInfo& non_mapped_info : nodes)
        non_mapped_info.reset_ply();
    for (Info::nodes_type& nodes : *white_to_move_infos_)
      for (Info& info : nodes)
        info.reset_ply();
    for (NonMappedInfo::nodes_type& nodes : *white_to_move_non_mapped_infos_)
      for (NonMappedInfo& non_mapped_info : nodes)
        non_mapped_info.reset_ply();
  }

  void classify();

  template<color_type to_move>
  Info& get_info(Board board)
  {
    infos_type& infos = to_move == black ? *black_to_move_infos_ : *white_to_move_infos_;
    return infos[board.as_partition()][board.as_partition_element()];
  }

  template<color_type to_move>
  std::tuple<Info&, NonMappedInfo&> get_info_tuple(Board board)
  {
    infos_type& infos =
      to_move == black ? *black_to_move_infos_            : *white_to_move_infos_;
    non_mapped_infos_type& non_mapped_infos =
      to_move == black ? *black_to_move_non_mapped_infos_ : *white_to_move_non_mapped_infos_;

    return {     infos[board.as_partition()][board.as_partition_element()],
      non_mapped_infos[board.as_partition()][board.as_partition_element()] };
  }

  template<color_type to_move>
  Info const& get_info(Board board) const
  {
    infos_type const& infos = to_move == black ? *black_to_move_infos_ : *white_to_move_infos_;
    return infos[board.as_partition()][board.as_partition_element()];
  }

  template<color_type to_move>
  std::tuple<Info const&, NonMappedInfo const&> get_info_tuple(Board board) const
  {
    infos_type const& infos =
      to_move == black ? *black_to_move_infos_            : *white_to_move_infos_;
    non_mapped_infos_type const& non_mapped_infos =
      to_move == black ? *black_to_move_non_mapped_infos_ : *white_to_move_non_mapped_infos_;

    return {     infos[board.as_partition()][board.as_partition_element()],
      non_mapped_infos[board.as_partition()][board.as_partition_element()] };
  }

  template<color_type to_move>
  Info& get_info(Partition partition, PartitionElement partition_element)
  {
    infos_type& infos = to_move == black ? *black_to_move_infos_ : *white_to_move_infos_;
    return infos[partition][partition_element];
  }

  template<color_type to_move>
  Info const& get_info(Partition partition, PartitionElement partition_element) const
  {
    infos_type& infos = to_move == black ? *black_to_move_infos_ : *white_to_move_infos_;
    return infos[partition][partition_element];
  }

  // Returns a memory-mapped array (whose index is a PartitionIndex) that contains
  // arrays (whose index is an InfoIndex that contains) Info objects corresponding
  // to all Board that belong to the given Partition(Index), where black is to move.
  // The InfoIndex follows from the Board itself, so you need a Board (or InfoIndex)
  // to find the Info object, the other way is not possible. When running over all
  // InfoIndexes one has to take into account that those include illegal Board's.
  // If the corresponding Board is illegal then the returned Info object is invalid.
  infos_type const& black_to_move_infos() const
  {
    return *black_to_move_infos_;
  }

  // Same but with white to move for the given Board (if it is legal).
  infos_type const& white_to_move_infos() const
  {
    return *white_to_move_infos_;
  }

  static std::filesystem::path data_directory(std::filesystem::path const& prefix_directory);
  static std::filesystem::path data_filename(std::filesystem::path const& prefix_directory)
  {
    return Graph::data_directory(prefix_directory) / "mmap.img";
  }
};
