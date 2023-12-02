#include "fs.hpp"

class DatablockOperation {
public:
  char *buf;
  size_t count;
  size_t offset;
  size_t bytes_completed;
  RawDisk *disk;
  virtual int operation(u_int64_t block_num) = 0;
};

int Fs::sweep_inode_datablocks(INode_Data *inode_data,
                               u_int64_t start_block_index, bool allocate,
                               DatablockOperation *op) {
  int result;

  printf("%llu %llu %llu\n", NUMBER_OF_DIRECT_BLOCKS, INDIRECT_BLOCKS, INDIRECT_BLOCKS * INDIRECT_BLOCKS);

  u_int64_t start_index = start_block_index;
  for (size_t i = start_index; i < NUMBER_OF_DIRECT_BLOCKS; ++i) {
    if ((result = sweep_datablocks(&(inode_data->direct_blocks[i]), 0, 0,
                                   allocate, op)) <= 0)
      return result;
    start_index = NUMBER_OF_DIRECT_BLOCKS;
  }

  start_index -= NUMBER_OF_DIRECT_BLOCKS;

  if (start_index < INDIRECT_BLOCKS) {
    if ((result = sweep_datablocks(&(inode_data->single_indirect_block), 1,
                                   start_index, allocate, op)) <= 0)
      return result;
    start_index = INDIRECT_BLOCKS;
  }

  start_index -= INDIRECT_BLOCKS;

  if (start_index < INDIRECT_BLOCKS * INDIRECT_BLOCKS) {
    if ((result = sweep_datablocks(&(inode_data->double_indirect_block), 2,
                                   start_index, allocate, op)) <= 0)
      return result;
    start_index = INDIRECT_BLOCKS * INDIRECT_BLOCKS;
  }

  start_index -= INDIRECT_BLOCKS * INDIRECT_BLOCKS;

  if (start_index < (u_int64_t)INDIRECT_BLOCKS * INDIRECT_BLOCKS * INDIRECT_BLOCKS) {
    if ((result = sweep_datablocks(&(inode_data->triple_indirect_block), 3,
                                   start_index, allocate, op)) <= 0)
      return result;
  }

  return -1;
}

// This can simply be made non recursive by copy pasting - it is just
// written this way as a proof of concept
int Fs::sweep_datablocks(u_int64_t *block_num, int indirect_num,
                         u_int64_t start_block_index, bool allocate,
                         DatablockOperation *op) {
  char buf[IO_BLOCK_SIZE];
  int err;
  int result = -1;

  //printf("SWEEP %llu %d %llu %d\n", *block_num, indirect_num, start_block_index, int(allocate));

  if (allocate && (*block_num) == 0)
    if ((err = datablock_manager->new_datablock(block_num)) < 0)
      return err;
    
  if (indirect_num == 0)
    return op->operation(*block_num);

  if ((*block_num) == 0) {
    memset(buf, 0, sizeof(buf));
  } else {
    if ((err = disk->read_block(*block_num, buf)) < 0)
      return err;
  }
  

  u_int64_t indirect_block_size = 1;
  for (int i = 1; i < indirect_num; ++i)
    indirect_block_size *= INDIRECT_BLOCKS;

  u_int64_t this_layer_start_index = start_block_index / indirect_block_size;
  u_int64_t next_layer_start_index =
      start_block_index - (indirect_block_size * this_layer_start_index);

  u_int64_t temp;
  u_int64_t next_block_num;
  bool modified = false;
  //printf("SWEEP TO LOWER LEVEL %llu %llu %llu\n", this_layer_start_index, next_layer_start_index, indirect_block_size);

  for (size_t i = this_layer_start_index * sizeof(u_int64_t); i < IO_BLOCK_SIZE;
       i += sizeof(u_int64_t)) {
    read_u64(&temp, &buf[i]);
    next_block_num = temp;
    if ((result = sweep_datablocks(&next_block_num, indirect_num - 1,
                                   next_layer_start_index, allocate, op)) < 0)
      return result;
    if (next_block_num != temp) {
      write_u64(next_block_num, &buf[i]);
      modified = true;
    }
    if (result == 0)
      break;
  }
  if (modified)
    if ((err = disk->write_block(*block_num, buf)) < 0)
      return err;
  return result;
}

class ReadDatablockOperation : public DatablockOperation {
public:
  int operation(u_int64_t block_num) override {
    char datablock_buf[IO_BLOCK_SIZE];
    int err;

    // printf("PRINT: (%d) %d %d %d\n", block_num, count, offset,
    // bytes_completed);

    size_t read_size =
        std::min(IO_BLOCK_SIZE - offset, count - bytes_completed);

    if (block_num != 0) {
      if ((err = disk->read_block(block_num, datablock_buf)) < 0)
        return err;

      memcpy(&buf[bytes_completed], &datablock_buf[offset], read_size);
    } else {
      memset(&buf[bytes_completed], 0, read_size);
    }

    offset = 0;
    bytes_completed += read_size;

    if (bytes_completed >= count)
      return 0;
    return 1;
  }
};

class WriteDatablockOperation : public DatablockOperation {
public:
  int operation(u_int64_t block_num) override {
    char datablock_buf[IO_BLOCK_SIZE];
    int err;

    size_t write_size =
        std::min(IO_BLOCK_SIZE - offset, count - bytes_completed);

    if (write_size < IO_BLOCK_SIZE)
      if ((err = disk->read_block(block_num, datablock_buf)) < 0)
        return err;

    memcpy(&datablock_buf[offset], &buf[bytes_completed], write_size);

    if ((err = disk->write_block(block_num, datablock_buf)) < 0)
      return err;

    offset = 0;
    bytes_completed += write_size;

    if (bytes_completed >= count)
      return 0;
    return 1;
  }
};

ssize_t Fs::read(INode_Data *inode_data, char buf[], size_t count,
                 size_t offset) {
  int err;

  u_int64_t start_block_index = offset / IO_BLOCK_SIZE;
  size_t internal_offset = offset - (start_block_index * IO_BLOCK_SIZE);

  ReadDatablockOperation op = ReadDatablockOperation();
  op.offset = internal_offset;
  op.buf = buf;
  op.count = std::min(count, inode_data->metadata.size - offset);
  op.bytes_completed = 0;
  op.disk = disk;

  if ((err = sweep_inode_datablocks(inode_data, start_block_index, false,
                                    &op)) < 0)
    return err;

  return op.bytes_completed;
}

ssize_t Fs::write(INode_Data *inode_data, char buf[], size_t count,
                  size_t offset) {
  int err;

  u_int64_t start_block_index = offset / IO_BLOCK_SIZE;
  size_t internal_offset = offset - (start_block_index * IO_BLOCK_SIZE);

  WriteDatablockOperation op = WriteDatablockOperation();
  op.offset = internal_offset;
  op.buf = buf;
  op.count = count;
  op.bytes_completed = 0;
  op.disk = disk;

  printf("trying to write %d %llu %llu\n", op.count, offset, start_block_index);

  if ((err = sweep_inode_datablocks(inode_data, start_block_index, true, &op)) <
      0)
    return err;

  inode_data->metadata.size = 
      std::max(offset + op.bytes_completed, inode_data->metadata.size);

  printf("written %d\n", op.bytes_completed);

  return op.bytes_completed;
}