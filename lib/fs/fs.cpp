#include "fs.hpp"

Fs::Fs(RawDisk *disk) : disk(disk) {
  superblock = SuperBlock_Data();
  inode_manager = new INode_Manager_Freelist(this, 1, 1 + NUM_INODE_BLOCKS);
  datablock_manager =
      new DataBlock_Manager_Bitmap(this, 1 + NUM_INODE_BLOCKS, disk->diskSize/IO_BLOCK_SIZE);
};

Fs::~Fs() {
  delete inode_manager;
  delete datablock_manager;
};

int Fs::format() {
  int err;
  if ((err = save_superblock()) < 0)
    return err;
  if ((err = inode_manager->format()) < 0)
    return err;
  if ((err = datablock_manager->format()) < 0)
    return err;
  return 0;
}

int Fs::load_superblock() {
  char buf[IO_BLOCK_SIZE];
  int err;

  if ((err = disk->read_block(0, buf)) < 0)
    return err;

  superblock.deserialize(buf);

  return 0;
}
int Fs::save_superblock() {
  char buf[IO_BLOCK_SIZE] = {0};
  int err;

  superblock.serialize(buf);

  if ((err = disk->write_block(0, buf)) < 0)
    return err;

  return 0;
}

int Fs::save_free_list_head(u_int64_t new_free_list_head) {
  u_int64_t temp = superblock.free_list_head;
  int err;
  superblock.free_list_head = new_free_list_head;
  if ((err = save_superblock()) < 0) {
    superblock.free_list_head = temp;
    return err;
  }
  return 0;
}
int Fs::save_inode_list_head(u_int64_t new_inode_list_head) {
  u_int64_t temp = superblock.inode_list_head;
  int err;
  superblock.inode_list_head = new_inode_list_head;
  if ((err = save_superblock()) < 0) {
    superblock.inode_list_head = temp;
    return err;
  }
  return 0;
}