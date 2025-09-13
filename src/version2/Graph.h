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
  static constexpr size_t number_of_mutexes = 131072;
  using infos_type = utils::Array<Info::nodes_type, number_of_partitions, PartitionIndex>;
  using auxiliary_infos_type = utils::Array<AuxiliaryInfo::nodes_type, number_of_partitions, PartitionIndex>;
  using black_to_move_infos_type = std::unique_ptr<infos_type, std::function<void(infos_type*)>>;
  using black_to_move_auxiliary_infos_type = std::unique_ptr<auxiliary_infos_type, std::function<void(auxiliary_infos_type*)>>;
  using white_to_move_infos_type = std::unique_ptr<infos_type, std::function<void(infos_type*)>>;
  using white_to_move_auxiliary_infos_type = std::unique_ptr<auxiliary_infos_type, std::function<void(auxiliary_infos_type*)>>;

 private:
  std::filesystem::path data_directory_;
  memory::MemoryMappedPool infos_pool_;
  memory::MemoryMappedPool auxiliary_infos_pool_;
  bool reuse_file_;
  black_to_move_infos_type black_to_move_infos_;
  black_to_move_auxiliary_infos_type black_to_move_auxiliary_infos_;
  white_to_move_infos_type white_to_move_infos_;
  white_to_move_auxiliary_infos_type white_to_move_auxiliary_infos_;
  std::vector<std::mutex> mutexes_;

  // The space allocated for an `infos_type` array.
  // This is also the offset (in the memory mapped file) between the black_to_move_infos_ and the white_to_move_infos_ array.
  size_t infos_size()
  {
    size_t const memory_page_size = memory::MemoryMappedPool::memory_page_size();
    return utils::nearest_multiple_of_power_of_two(sizeof(infos_type), memory_page_size);
  }

  size_t auxiliary_infos_size()
  {
    size_t const memory_page_size = memory::MemoryMappedPool::memory_page_size();
    return utils::nearest_multiple_of_power_of_two(sizeof(auxiliary_infos_type), memory_page_size);
  }

  void do_allocate()
  {
    if (!reuse_file_)
    {
      void* black_to_move_infos_pool = infos_pool_.allocate();
      ASSERT(black_to_move_infos_pool != nullptr && black_to_move_infos_pool == black_to_move_infos_start());
      void* white_to_move_infos_pool = infos_pool_.allocate();
      ASSERT(white_to_move_infos_pool != nullptr && white_to_move_infos_pool == white_to_move_infos_start());
    }
    void* black_to_move_auxiliary_infos_pool = auxiliary_infos_pool_.allocate();
    ASSERT(black_to_move_auxiliary_infos_pool != nullptr && black_to_move_auxiliary_infos_pool == black_to_move_auxiliary_infos_start());
    void* white_to_move_auxiliary_infos_pool = auxiliary_infos_pool_.allocate();
    ASSERT(white_to_move_auxiliary_infos_pool != nullptr && white_to_move_auxiliary_infos_pool == white_to_move_auxiliary_infos_start());
  }

  void* black_to_move_infos_start()
  {
    return infos_pool_.mapped_base();
  }

  void* white_to_move_infos_start()
  {
    return static_cast<char*>(infos_pool_.mapped_base()) + infos_size();
  }

  void* black_to_move_auxiliary_infos_start()
  {
    return auxiliary_infos_pool_.mapped_base();
  }

  void* white_to_move_auxiliary_infos_start()
  {
    return static_cast<char*>(auxiliary_infos_pool_.mapped_base()) + auxiliary_infos_size();
  }

 public:
  Graph(std::filesystem::path prefix_directory, bool reuse_file, bool read_only = false) :
    data_directory_(data_directory(prefix_directory)),
    infos_pool_(data_filename(prefix_directory), infos_size(), 2 * infos_size(),
        read_only ? memory::MemoryMappedPool::Mode::copy_on_write : memory::MemoryMappedPool::Mode::persistent, !reuse_file),
    auxiliary_infos_pool_(tmp_data_filename(prefix_directory), auxiliary_infos_size(), 2 * auxiliary_infos_size(),
        memory::MemoryMappedPool::Mode::persistent, true),
    reuse_file_(reuse_file),
    mutexes_(number_of_mutexes)
    {
      do_allocate();
      black_to_move_infos_ = black_to_move_infos_type(
          new (black_to_move_infos_start()) infos_type, [this](infos_type* ptr){ });
      black_to_move_auxiliary_infos_ = black_to_move_auxiliary_infos_type(
          new (black_to_move_auxiliary_infos_start()) auxiliary_infos_type, [this](auxiliary_infos_type* ptr){ });
      white_to_move_infos_ = white_to_move_infos_type(
          new (white_to_move_infos_start()) infos_type, [this](infos_type* ptr){ });
      white_to_move_auxiliary_infos_ = white_to_move_auxiliary_infos_type(
          new (white_to_move_auxiliary_infos_start()) auxiliary_infos_type, [this](auxiliary_infos_type* ptr){ });
    }

  ~Graph()
  {
    DoutEntering(dc::notice, "Graph::~Graph()");
    std::filesystem::path tmp_data = data_directory_ / "tmp_data.img";
    Dout(dc::notice, "Removing " << tmp_data);
    std::filesystem::remove(tmp_data);
  }

  void initialize()
  {
    // This sets everything to zero.
    for (Info::nodes_type& nodes : *black_to_move_infos_)
      for (Info& info : nodes)
        info.initialize();
    for (AuxiliaryInfo::nodes_type& nodes : *black_to_move_auxiliary_infos_)
      for (AuxiliaryInfo& auxiliary_info : nodes)
        auxiliary_info.initialize();
    for (Info::nodes_type& nodes : *white_to_move_infos_)
      for (Info& info : nodes)
        info.initialize();
    for (AuxiliaryInfo::nodes_type& nodes : *white_to_move_auxiliary_infos_)
      for (AuxiliaryInfo& auxiliary_info : nodes)
        auxiliary_info.initialize();
  }

  void reset_ply()
  {
    // This sets everything required to recalculate the ply to zero.
    for (Info::nodes_type& nodes : *black_to_move_infos_)
      for (Info& info : nodes)
        info.reset_ply();
    for (AuxiliaryInfo::nodes_type& nodes : *black_to_move_auxiliary_infos_)
      for (AuxiliaryInfo& auxiliary_info : nodes)
        auxiliary_info.reset_ply();
    for (Info::nodes_type& nodes : *white_to_move_infos_)
      for (Info& info : nodes)
        info.reset_ply();
    for (AuxiliaryInfo::nodes_type& nodes : *white_to_move_auxiliary_infos_)
      for (AuxiliaryInfo& auxiliary_info : nodes)
        auxiliary_info.reset_ply();
  }

  void classify();

  template<color_type to_move>
  Info& get_info(Board board)
  {
    infos_type& infos = to_move == black ? *black_to_move_infos_ : *white_to_move_infos_;
    return infos[board.as_partition()][board.as_partition_element()];
  }

  template<color_type to_move>
  std::tuple<Info&, AuxiliaryInfo&> get_info_tuple(Board board)
  {
    infos_type& infos =
      to_move == black ? *black_to_move_infos_            : *white_to_move_infos_;
    auxiliary_infos_type& auxiliary_infos =
      to_move == black ? *black_to_move_auxiliary_infos_ : *white_to_move_auxiliary_infos_;

    return {     infos[board.as_partition()][board.as_partition_element()],
      auxiliary_infos[board.as_partition()][board.as_partition_element()] };
  }

  template<color_type to_move>
  Info const& get_info(Board board) const
  {
    infos_type const& infos = to_move == black ? *black_to_move_infos_ : *white_to_move_infos_;
    return infos[board.as_partition()][board.as_partition_element()];
  }

  template<color_type to_move>
  std::tuple<Info const&, AuxiliaryInfo const&> get_info_tuple(Board board) const
  {
    infos_type const& infos =
      to_move == black ? *black_to_move_infos_            : *white_to_move_infos_;
    auxiliary_infos_type const& auxiliary_infos =
      to_move == black ? *black_to_move_auxiliary_infos_ : *white_to_move_auxiliary_infos_;

    return {     infos[board.as_partition()][board.as_partition_element()],
      auxiliary_infos[board.as_partition()][board.as_partition_element()] };
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

  std::mutex& get_mutex(Board board)
  {
    return mutexes_[board.get_encoded() % number_of_mutexes];
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
  static std::filesystem::path tmp_data_filename(std::filesystem::path const& prefix_directory)
  {
    return Graph::data_directory(prefix_directory) / "tmp_data.img";
  }
};
