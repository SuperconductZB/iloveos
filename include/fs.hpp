#ifndef FS_HPP
#define FS_HPP

#include "fs/datablock_manager.hpp"
#include "fs/fs_data_types.hpp"
#include "fs/inode_manager.hpp"
#include "fs_constants.hpp"
#include "rawdisk.hpp"

// TEMP:
class DatablockOperation;

class Fs {
public:
  Fs(RawDisk *disk);
  ~Fs();

  ssize_t read(INode_Data *inode_data, char buf[], size_t count, size_t offset);
  ssize_t write(INode_Data *inode_data, const char buf[], size_t count,
                size_t offset);
  int truncate(INode_Data *inode_data, off_t length);
  ssize_t lseek_next_data(INode_Data *inode_data, size_t offset);
  ssize_t lseek_next_hole(INode_Data *inode_data, size_t offset);

  int format();

  // should probably be private but is not for testing
  RawDisk *disk;
  SuperBlock_Data superblock;
  INode_Manager *inode_manager;
  DataBlock_Manager *datablock_manager;

  int load_superblock();
  int save_superblock();

  int save_free_list_head(u_int64_t new_free_list_head);
  int save_inode_list_head(u_int64_t new_inode_list_head);

  int sweep_inode_datablocks(INode_Data *inode_data,
                             u_int64_t start_block_index, bool allocate,
                             DatablockOperation *op);

  int sweep_datablocks(u_int64_t *block_num, int indirect_num,
                       u_int64_t start_block_index, bool allocate,
                       DatablockOperation *op);
};

#endif