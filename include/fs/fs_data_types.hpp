#ifndef FS_DATA_TYPES_HPP
#define FS_DATA_TYPES_HPP

#include "fs_constants.hpp"

size_t write_u64(u_int64_t num, char buf[]);

size_t read_u64(u_int64_t *num, char buf[]);

size_t write_u32(u_int32_t num, char buf[]);

size_t read_u32(u_int32_t *num, char buf[]);

class SuperBlock_Data {
public:
  u_int64_t free_list_head;
  u_int64_t inode_list_head;
  SuperBlock_Data();
  void serialize(char buf[]);
  void deserialize(char buf[]);
};

class INode_Data {
public:
  u_int64_t inode_num;

#define NUMBER_OF_METADATA_BYTES                                               \
  (6 * sizeof(u_int64_t) + (2 * sizeof(u_int32_t)))
  struct INode_MetaData {
    u_int64_t uid;
    u_int64_t gid;
    u_int64_t permissions;
    u_int64_t size; // not yet implemented
    u_int64_t access_time;
    u_int64_t modification_time;
    u_int32_t reference_count;
    u_int32_t flags;
  } metadata;
  size_t serialize_metadata(char buf[]);
  size_t deserialize_metadata(char buf[]);

#define NUMBER_OF_DIRECT_BLOCKS                                                \
  (((INODE_SIZE - NUMBER_OF_METADATA_BYTES) / sizeof(u_int64_t)) - 3)
#define FILE_SIZE_MAX                                                          \
  (IO_BLOCK_SIZE * (NUMBER_OF_DIRECT_BLOCKS + INDIRECT_BLOCKS +                \
                    (INDIRECT_BLOCKS * INDIRECT_BLOCKS) +                      \
                    (INDIRECT_BLOCKS * INDIRECT_BLOCKS * INDIRECT_BLOCKS)))

  u_int64_t single_indirect_block, double_indirect_block, triple_indirect_block;
  u_int64_t direct_blocks[NUMBER_OF_DIRECT_BLOCKS];

  INode_Data(u_int64_t inode_num = (u_int64_t)(0xFFFFFFFFFFFFFFFF));
  void serialize(char buf[]);
  void deserialize(char buf[]);
};

#endif