#ifndef INODE_ALLOCATOR_HPP
#define INODE_ALLOCATOR_HPP

#include "fs_data_types.hpp"
#include "rawdisk.hpp"

class Fs;

class INode_Allocator {
public:
  const int INODES_PER_BLOCK = BLOCK_SIZE / INODE_SIZE;

  INode_Allocator(Fs *fs, u_int64_t block_segment_start,
                  u_int64_t block_segment_end);

  virtual int new_inode(u_int64_t uid, u_int64_t gid, u_int64_t permissions,
                        INode_Data *inode_data) = 0;
  virtual int free_inode(INode_Data *inode_data) = 0;

  virtual int format() = 0;

  u_int64_t get_block_num(INode_Data *inode_data);
  u_int64_t get_block_offset(INode_Data *inode_data);

protected:
  Fs *fs;
  u_int64_t block_segment_start, block_segment_end;
  u_int64_t max_num_inodes;
};

class INode_Allocator_Freelist : INode_Allocator {
  using INode_Allocator::INode_Allocator;

  int new_inode(u_int64_t uid, u_int64_t gid, u_int64_t permissions,
                INode_Data *inode_data) override;
  int free_inode(INode_Data *inode_data) override;

  int format() override;
};

#endif