#include "fs.hpp"

INode_Allocator::INode_Allocator(Fs *fs, u_int64_t block_segment_start,
                                 u_int64_t block_segment_end) {
  fs = fs;
  block_segment_start = block_segment_start;
  block_segment_end = block_segment_end;
}

u_int64_t INode_Allocator::get_block_num(INode_Data *inode_data) {
  u_int64_t block_num =
      block_segment_start + (inode_data->inode_num / INODES_PER_BLOCK);
  if (block_num >= block_segment_end)
    return 0;
  return block_num;
}
u_int64_t INode_Allocator::get_block_offset(INode_Data *inode_data) {
  return (inode_data->inode_num % INODES_PER_BLOCK) * INODE_SIZE;
}

int INode_Allocator_Freelist::new_inode(u_int64_t uid, u_int64_t gid,
                                        u_int64_t permissions,
                                        INode_Data *inode_data) {
  (*inode_data) = nullptr;
  return -1;
}
int INode_Allocator_Freelist::free_inode(INode_Data *inode_data) { return -1; }

int INode_Allocator_Freelist::format() { return -1; }
