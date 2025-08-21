#pragma once

#include "BlockIndex.h"
#include "utils/VectorIndex.h"
#include "utils/square.h"

class Partition;
using PartitionIndex = utils::VectorIndex<Partition>;

class Partition
{
 private:
  PartitionIndex index_;

 public:
  Partition(BlockIndex black_king, BlockIndex white_king) :
    index_(white_king.index() + BlockIndex::number_of_blocks * black_king.index()) { }

  Partition(PartitionIndex partition_index) : index_(partition_index) { }

  Partition& operator++() { ++index_; return *this; }

  // Accessor.
  operator PartitionIndex() const { return index_; }

  BlockIndex black_king_block_index() const { return static_cast<BlockIndex::index_type>(index_.get_value() / BlockIndex::number_of_blocks); }
  BlockIndex white_king_block_index() const { return index_.get_value() % BlockIndex::number_of_blocks; }

  friend bool operator!=(Partition lhs, Partition rhs) { return lhs.index_ != rhs.index_; }
};
