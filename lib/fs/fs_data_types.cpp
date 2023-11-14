#include "fs.hpp"

size_t write_u64(u_int64_t num, char buf[]) {
  size_t i = 0;
  for (; i < 8; ++i)
    buf[i] = (char)(num >> (i * 8));
  return i;
}

size_t read_u64(u_int64_t *num, char buf[]) {
  size_t i = 0;
  (*num) = 0;
  for (; i < 8; ++i) {
    (*num) <<= 8;
    (*num) |= ((u_int64_t)buf[i]) & 0xFF;
  }
  return i;
}

SuperBlock_Data::SuperBlock_Data() {
  free_list_head = 0;
  inode_list_head = 0;
}

SuperBlock_Data::serialize(char buf[]) {
  size_t i = 0;
  i += write_u64(free_list_head, &buf[i]);
  i += write_u64(inode_list_head, &buf[i]);
}

SuperBlock_Data::deserialize(char buf[]) {
  size_t i = 0;
  i += read_u64(&free_list_head, &buf[i]);
  i += read_u64(&inode_list_head, &buf[i]);
}

INode_Data::INode_Data(u_int64_t inode_num) {
  inode_num = inode_num;

  metadata.uid = -1;
  metadata.gid = -1;
  metadata.permissions = -1;
  metadata.size = 0;
  metadata.reference_count = 0;

  single_indirect_block = double_indirect_block = triple_indirect_block = 0;

  for (size_t i = 0; i < NUMBER_OF_DIRECT_BLOCKS; ++i)
    direct_blocks[i] = 0;
}

size_t INode_Data::serialize_metadata(char buf[]) {
  size_t i = 0;
  i += write_u64(metadata.uid, &buf[i]);
  i += write_u64(metadata.gid, &buf[i]);
  i += write_u64(metadata.permissions, &buf[i]);
  i += write_u64(metadata.size, &buf[i]);
  i += write_u64(metadata.reference_count, &buf[i]);
  return i;
}

size_t INode_Data::deserialize_metadata(char buf[]) {
  size_t i = 0;
  i += read_u64(&metadata.uid, &buf[i]);
  i += read_u64(&metadata.gid, &buf[i]);
  i += read_u64(&metadata.permissions, &buf[i]);
  i += read_u64(&metadata.size, &buf[i]);
  i += read_u64(&metadata.reference_count, &buf[i]);
  return i;
}

void INode_Data::serialize(char buf[]) {
  size_t i = 0;
  i += serialize_metadata(&buf[i]);
  i += write_u64(triple_indirect_block, &buf[i]);
  i += write_u64(double_indirect_block, &buf[i]);
  i += write_u64(single_indirect_block, &buf[i]);
  for (size_t j = 0; j < NUMBER_OF_DIRECT_BLOCKS; ++j)
    i += write_u64(direct_blocks[j], &buf[i]);
}
void INode_Data::deserialize(char buf[]) {
  size_t i = 0;
  i += deserialize_metadata(&buf[i]);
  i += read_u64(&triple_indirect_block, &buf[i]);
  i += read_u64(&double_indirect_block, &buf[i]);
  i += read_u64(&single_indirect_block, &buf[i]);
  for (size_t j = 0; j < NUMBER_OF_DIRECT_BLOCKS; ++j)
    i += read_u64(&direct_blocks[j], &buf[i]);
}