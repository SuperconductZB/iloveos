#ifndef FS_HPP
#define FS_HPP

#include "fs/datablock_allocator.hpp"
#include "fs/fs_data_types.hpp"
#include "fs/inode_allocator.hpp"
#include "fs_constants.hpp"
#include "rawdisk.hpp"

class Fs {
public:
  Fs(RawDisk *disk);
  ~Fs();

  int allocate_datablock(INode_Data *inode_data);
  int deallocate_datablock(INode_Data *inode_data);

  int format();

  // should probably be private but is not for testing
  RawDisk *disk;
  SuperBlock_Data superblock;
  INode_Allocator *inode_allocator;
  DataBlock_Allocator *datablock_allocator;

  int load_superblock();
  int save_superblock();

  int save_free_list_head(u_int64_t new_free_list_head);
  int save_inode_list_head(u_int64_t new_inode_list_head);

  int load_inode(INode_Data *inode_data);
  int save_inode(INode_Data *inode_data);

  int allocate_indirect(u_int64_t *storage, int n);
  int deallocate_indirect(u_int64_t *storage, int n);
};

#endif