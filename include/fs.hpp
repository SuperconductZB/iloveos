#ifndef FS_HPP
#define FS_HPP

#include "fs/datablock_allocator.hpp"
#include "fs/fs_data_types.hpp"
#include "fs/inode_allocator.hpp"
#include "rawdisk.hpp"

#define NUM_INODE_BLOCKS 1023
#define NUM_BLOCKS 2048

class Fs {
public:
  Fs(RawDisk *disk);

  int resize(INode_Data *inode_data, u_int64_t size, bool absolute);

  int format();

  // should probably be private but is not for testing
  RawDisk *disk;
  SuperBlock_Data superblock;
  INode_Allocator inode_allocator;
  DataBlock_Allocator datablock_allocator;

  int load_superblock();
  int store_superblock();

  int load_inode(INode_Data *inode_data);
  int store_inode(INode_Data *inode_data);
};

#endif