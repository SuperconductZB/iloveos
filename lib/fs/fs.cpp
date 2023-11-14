#include "fs.hpp"

Fs::Fs(RawDisk *disk) {
  disk = disk;
  superblock = SuperBlock_Data();
  inode_allocator = INode_Allocator_Freelist(this, 1, 1 + NUM_INODE_BLOCKS);
  datablock_allocator =
      DataBlock_Allocator_Bitmap(this, 1 + NUM_INODE_BLOCKS, NUM_BLOCKS);
};

int Fs::format() {
  int err;
  if ((err = store_superblock()) < 0)
    return err;
  if ((err = inode_allocator.format()) < 0)
    return err;
  if ((err = datablock_allocator.format()) < 0)
    return err;
  return 0;
}

int Fs::load_superblock() {
  char buf[BLOCK_SIZE];
  int err;

  if ((err = disk->read_block(0, buf)) < 0)
    return err;

  superblock.deserialize(buf);

  return 0;
}
int Fs::store_superblock() {
  char buf[BLOCK_SIZE] = {0};
  int err;

  superblock.serialize(buf);

  if ((err = disk->write_block(0, buf)) < 0)
    return err;

  return 0;
}

int Fs::load_inode(INode_Data *inode_data) {
  char buf[BLOCK_SIZE];
  int err;

  u_int64_t block_num = inode_allocator.get_block_num(inode_data);
  if (block_num == 0)
    return -1;
  u_int64_t block_offset = inode_allocator.get_block_offset(inode_data);

  if ((err = disk->read_block(block_num, buf)) < 0)
    return err;

  inode_data->deserialize(&buf[block_offset])

      return 0;
}
int Fs::store_inode(INode_Data *inode_data) {
  char buf[BLOCK_SIZE];
  int err;

  u_int64_t block_num = inode_allocator.get_block_num(inode_data);
  if (block_num == 0)
    return -1;
  u_int64_t block_offset = inode_allocator.get_block_offset(inode_data);

  if ((err = disk->read_block(block_num, buf)) < 0)
    return err;

  inode_data->serialize(&buf[block_offset])

      if ((err = disk->write_block(block_num, buf)) < 0) return err;

  return 0;
}