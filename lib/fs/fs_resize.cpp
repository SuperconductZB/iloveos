#include "fs.hpp"

int Fs::deallocate_datablock(INode_Data *inode_data, u_int64_t *datablock_num) {
  int result;

  result = deallocate_indirect(&(inode_data->triple_indirect_block), 3,
                               datablock_num);
  if (result <= 0)
    return result;

  result = deallocate_indirect(&(inode_data->double_indirect_block), 2,
                               datablock_num);
  if (result <= 0)
    return result;

  result = deallocate_indirect(&(inode_data->single_indirect_block), 1,
                               datablock_num);
  if (result <= 0)
    return result;

  for (size_t i = NUMBER_OF_DIRECT_BLOCKS - 1; i >= 0; --i) {
    result =
        deallocate_indirect(&(inode_data->direct_blocks[i]), 0, datablock_num);
    if (result <= 0)
      return result;
  }

  return -1;
}

int Fs::deallocate_indirect(u_int64_t *storage, int n,
                            u_int64_t *datablock_num) {
  char buf[IO_BLOCK_SIZE];
  int result;

  if (*storage == 0)
    return 1;

  if (n == 0) {
    u_int64_t temp_datablock_num = (*storage);
    if ((result = datablock_manager->free_datablock(*storage)) < 0)
      return result;
    (*datablock_num) = temp_datablock_num;
    (*storage) = 0;
    return 0;
  }

  u_int64_t temp;

  if ((result = disk->read_block(*storage, buf)) < 0)
    return result;

  for (size_t i = IO_BLOCK_SIZE - sizeof(u_int64_t); i >= 0;
       i -= sizeof(u_int64_t)) {
    read_u64(&temp, &buf[i]);
    result = deallocate_indirect(&temp, n - 1, datablock_num);
    if (result < 0)
      return result;
    if (result == 0) {
      if (i == 0 && temp == 0) {
        if ((result = datablock_manager->free_datablock(*storage)) < 0)
          return result;
        (*storage) = 0;
      } else {
        write_u64(temp, &buf[i]);
        if ((result = disk->write_block(*storage, buf)) < 0)
          return result;
      }
      return 0;
    }
  }

  return 1;
}