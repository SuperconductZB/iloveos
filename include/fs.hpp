#ifndef FS_HPP
#define FS_HPP

#include "fs/datablock_manager.hpp"
#include "fs/fs_data_types.hpp"
#include "fs/inode_manager.hpp"
#include "fs_constants.hpp"
#include "rawdisk.hpp"

class Fs {
public:
  Fs(RawDisk *disk);
  ~Fs();

  int allocate_datablock(INode_Data *inode_data, u_int64_t *datablock_num);
  int deallocate_datablock(INode_Data *inode_data, u_int64_t *datablock_num);

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

  int allocate_indirect(u_int64_t *storage, int n, u_int64_t *datablock_num);
  int deallocate_indirect(u_int64_t *storage, int n, u_int64_t *datablock_num);
};

#endif