#include "fs.hpp"

INode_Manager::INode_Manager(Fs *fs, u_int64_t block_segment_start,
                                 u_int64_t block_segment_end)
    : fs(fs), block_segment_start(block_segment_start),
      block_segment_end(block_segment_end) {
  max_num_inodes = (block_segment_end - block_segment_start) * INODES_PER_BLOCK;
}

u_int64_t INode_Manager::get_block_num(u_int64_t inode_num) {
  u_int64_t block_num = block_segment_start + (inode_num / INODES_PER_BLOCK);
  if (block_num >= block_segment_end)
    return 0;
  return block_num;
}
u_int64_t INode_Manager::get_block_offset(u_int64_t inode_num) {
  return (inode_num % INODES_PER_BLOCK) * INODE_SIZE;
}

int INode_Manager::load_inode(INode_Data *inode_data) {
  char buf[IO_BLOCK_SIZE];
  int err;

  u_int64_t block_num = get_block_num(inode_data->inode_num);
  if (block_num == 0)
    return -1;
  u_int64_t block_offset = get_block_offset(inode_data->inode_num);

  if ((err = fs->disk->read_block(block_num, buf)) < 0)
    return err;

  inode_data->deserialize(&buf[block_offset]);

  return 0;
}
int INode_Manager::save_inode(INode_Data *inode_data) {
  char buf[IO_BLOCK_SIZE];
  int err;

  u_int64_t block_num = get_block_num(inode_data->inode_num);
  if (block_num == 0)
    return -1;
  u_int64_t block_offset = get_block_offset(inode_data->inode_num);

  if ((err = fs->disk->read_block(block_num, buf)) < 0)
    return err;

  inode_data->serialize(&buf[block_offset]);

  if ((err = fs->disk->write_block(block_num, buf)) < 0)
    return err;

  return 0;
}

int INode_Manager_Freelist::new_inode(u_int64_t uid, u_int64_t gid,
                                        u_int64_t permissions,
                                        INode_Data *inode_data) {
  char buf[IO_BLOCK_SIZE];
  int err;
  u_int64_t inode_num = fs->superblock.inode_list_head;
  if (inode_num > max_num_inodes)
    return -1;

  u_int64_t block_num = get_block_num(inode_num);
  u_int64_t block_offset = get_block_offset(inode_num);

  if (block_num == 0)
    return -1;

  if ((err = fs->disk->read_block(block_num, buf)) < 0)
    return err;

  u_int64_t new_inode_list_head = 0;
  read_u64(&new_inode_list_head, &buf[block_offset]);
  if ((err = fs->save_inode_list_head(new_inode_list_head)) < 0)
    return err;

  (*inode_data) = INode_Data(inode_num);

  inode_data->metadata.uid = uid;
  inode_data->metadata.gid = gid;
  inode_data->metadata.permissions = permissions;

  // It is debatable if this function should do this:
  if ((err = save_inode(inode_data)) < 0) {
    inode_data->inode_num = 0xFFFFFFFFFFFFFFFF;
    return err;
  }

  return 0;
}
int INode_Manager_Freelist::free_inode(INode_Data *inode_data) {
  char buf[IO_BLOCK_SIZE];
  int err;

  u_int64_t block_num = get_block_num(inode_data->inode_num);
  u_int64_t block_offset = get_block_offset(inode_data->inode_num);

  if (block_num == 0)
    return -1;

  if ((err = fs->disk->read_block(block_num, buf)) < 0)
    return err;

  write_u64(fs->superblock.inode_list_head, &buf[block_offset]);

  if ((err = fs->disk->write_block(block_num, buf)) < 0)
    return err;

  if ((err = fs->save_inode_list_head(inode_data->inode_num)) < 0)
    return err;

  return 0;
}

int INode_Manager_Freelist::format() {
  char buf[IO_BLOCK_SIZE];
  int err;
  u_int64_t next_inode_num = 1;
  for (u_int64_t i = block_segment_start; i < block_segment_end; ++i) {
    for (int j = 0; j < INODES_PER_BLOCK; ++next_inode_num, ++j)
      write_u64(next_inode_num, &buf[j * INODE_SIZE]);
    if ((err = fs->disk->write_block(i, buf)) < 0)
      return err;
  }
  if ((err = fs->save_inode_list_head(0)) < 0)
    return err;
  return 0;
}


