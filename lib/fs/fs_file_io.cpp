#include "fs.hpp"

class DatablockOperation {
public:
  DatablockOperation(int (*_skip)(DatablockOperation *, u_int64_t) = nullptr)
      : skip(_skip) {}
  size_t count;
  size_t offset;
  size_t bytes_completed;
  Fs *fs;
  virtual int operation(u_int64_t block_num, bool *delete_block) = 0;
  int (*skip)(DatablockOperation *, u_int64_t);
};

int default_skip_func(DatablockOperation *this_op, u_int64_t num_blocks) {
  this_op->bytes_completed += (num_blocks * IO_BLOCK_SIZE) - this_op->offset;
  this_op->offset = 0;

  if (this_op->bytes_completed >= this_op->count)
    return 0;
  return 1;
}

int truncate_skip_func(DatablockOperation *this_op, u_int64_t num_blocks) {
  this_op->offset = 0;
  return 1;
}

int Fs::sweep_inode_datablocks(INode_Data *inode_data,
                               u_int64_t start_block_index, bool allocate,
                               DatablockOperation *op) {
  int result;

  // printf("test2.1\n");

  u_int64_t start_index = start_block_index;
  for (size_t i = start_index; i < NUMBER_OF_DIRECT_BLOCKS; ++i) {
    if ((result = sweep_datablocks(&(inode_data->direct_blocks[i]), 0, 0,
                                   allocate, op)) <= 0)
      return result;
    start_index = NUMBER_OF_DIRECT_BLOCKS;
  }

  // printf("test2.2\n");

  start_index -= NUMBER_OF_DIRECT_BLOCKS;

  if (start_index < INDIRECT_BLOCKS) {
    if ((result = sweep_datablocks(&(inode_data->single_indirect_block), 1,
                                   start_index, allocate, op)) <= 0)
      return result;
    start_index = INDIRECT_BLOCKS;
  }

  // printf("test2.3\n");

  start_index -= INDIRECT_BLOCKS;

  if (start_index < INDIRECT_BLOCKS * INDIRECT_BLOCKS) {
    if ((result = sweep_datablocks(&(inode_data->double_indirect_block), 2,
                                   start_index, allocate, op)) <= 0)
      return result;
    start_index = INDIRECT_BLOCKS * INDIRECT_BLOCKS;
  }

  // printf("test2.4\n");

  start_index -= INDIRECT_BLOCKS * INDIRECT_BLOCKS;

  if (start_index <
      (u_int64_t)INDIRECT_BLOCKS * INDIRECT_BLOCKS * INDIRECT_BLOCKS) {
    if ((result = sweep_datablocks(&(inode_data->triple_indirect_block), 3,
                                   start_index, allocate, op)) <= 0)
      return result;
  }

  return 1;
}

// This can simply be made non recursive by copy pasting - it is just
// written this way as a proof of concept
int Fs::sweep_datablocks(u_int64_t *block_num, int indirect_num,
                         u_int64_t start_block_index, bool allocate,
                         DatablockOperation *op) {
  char buf[IO_BLOCK_SIZE];
  int err;
  int result = -1;

  u_int64_t num_blocks_indirect;
  u_int64_t num_blocks = 1;
  for (int i = 0; i < indirect_num; ++i) {
    num_blocks_indirect = num_blocks;
    num_blocks *= INDIRECT_BLOCKS;
  }

  // printf("test2.3.1 %d\n", indirect_num);

  if ((*block_num) == 0) {
    if (allocate) {
      if ((err = datablock_manager->new_datablock(block_num)) < 0)
        return err;
    } else if (op->skip != nullptr) {
      return (*(op->skip))(op, num_blocks);
    }
  }

  // printf("test2.3.2 %d\n", indirect_num);

  if (indirect_num == 0) {
    bool delete_block = false;
    if ((result = op->operation(*block_num, &delete_block)) < 0)
      return result;
    if (delete_block) {
      if ((err = datablock_manager->free_datablock(*block_num)) < 0)
        return err;
      (*block_num) = 0;
    }
    return result;
  }

  // printf("test2.3.3 %d\n", indirect_num);

  if ((*block_num) == 0) {
    memset(buf, 0, sizeof(buf));
  } else {
    if ((err = disk->read_block(*block_num, buf)) < 0)
      return err;
  }

  // printf("test2.3.4 %d\n", indirect_num);

  u_int64_t this_layer_start_index = start_block_index / num_blocks_indirect;
  u_int64_t next_layer_start_index =
      start_block_index - (num_blocks_indirect * this_layer_start_index);

  u_int64_t temp;
  u_int64_t next_block_num;
  bool modified = false;

  // printf("test2.3.4- %d\n", indirect_num);

  // printf("start_block_index=%d\n", start_block_index);
  // printf("this_layer_start_index=%d\n", this_layer_start_index);
  // printf("next_layer_start_index=%d\n", next_layer_start_index);
  // printf("num_blocks_indirect=%d\n", num_blocks_indirect);

  for (size_t i = this_layer_start_index * sizeof(u_int64_t); i < IO_BLOCK_SIZE;
       i += sizeof(u_int64_t)) {
    // printf("test2.3.5- %d\n", indirect_num);
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

  // printf("test2.3.6 %d\n", indirect_num);

  if (modified) {
    bool delete_block = true;
    for (size_t i = 0; i < IO_BLOCK_SIZE; ++i)
      if (buf[i] != 0) {
        delete_block = false;
        break;
      }
    if (delete_block) {
      if ((err = datablock_manager->free_datablock(*block_num)) < 0)
        return err;
      (*block_num) = 0;
    } else {
      if ((err = disk->write_block(*block_num, buf)) < 0)
        return err;
    }
  }

  // printf("test2.3.7 %d\n", indirect_num);
  // printf("test2.3.8 result=%d %d\n", result, indirect_num);

  return result;
}

class ReadDatablockOperation : public DatablockOperation {
public:
  char *buf;
  ReadDatablockOperation() : DatablockOperation() {}
  int operation(u_int64_t block_num, bool *delete_block) override {
    char datablock_buf[IO_BLOCK_SIZE];
    int err;

    // printf("PRINT: (%d) %d %d %d\n", block_num, count, offset,
    // bytes_completed);

    size_t read_size =
        std::min(IO_BLOCK_SIZE - offset, count - bytes_completed);

    if (block_num != 0) {
      if ((err = fs->disk->read_block(block_num, datablock_buf)) < 0)
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
  const char *buf;
  WriteDatablockOperation() : DatablockOperation() {}
  int operation(u_int64_t block_num, bool *delete_block) override {
    char datablock_buf[IO_BLOCK_SIZE];
    int err;

    // printf("w: %d\n", bytes_completed);

    size_t write_size =
        std::min(IO_BLOCK_SIZE - offset, count - bytes_completed);

    if (write_size < IO_BLOCK_SIZE)
      if ((err = fs->disk->read_block(block_num, datablock_buf)) < 0)
        return err;

    memcpy(&datablock_buf[offset], &buf[bytes_completed], write_size);

    if ((err = fs->disk->write_block(block_num, datablock_buf)) < 0)
      return err;

    offset = 0;
    bytes_completed += write_size;

    if (bytes_completed >= count)
      return 0;
    return 1;
  }
};

class TruncateDatablockOperation : public DatablockOperation {
public:
  TruncateDatablockOperation() : DatablockOperation(truncate_skip_func) {}
  int operation(u_int64_t block_num, bool *delete_block) override {
    char datablock_buf[IO_BLOCK_SIZE];
    int err;

    if (offset == 0) {
      (*delete_block) = true;
      return 1;
    }

    if ((err = fs->disk->read_block(block_num, datablock_buf)) < 0)
      return err;

    memset(&datablock_buf[offset], 0, IO_BLOCK_SIZE - offset);

    if ((err = fs->disk->write_block(block_num, datablock_buf)) < 0)
      return err;

    offset = 0;

    return 1;
  }
};

class LseekNextDataDatablockOperation : public DatablockOperation {
public:
  LseekNextDataDatablockOperation() : DatablockOperation(default_skip_func) {}
  int operation(u_int64_t block_num, bool *delete_block) override { return 0; }
};

class LseekNextHoleDatablockOperation : public DatablockOperation {
public:
  LseekNextHoleDatablockOperation() : DatablockOperation() {}
  int operation(u_int64_t block_num, bool *delete_block) override {
    if (block_num == 0)
      return 0;

    bytes_completed += (IO_BLOCK_SIZE)-offset;
    offset = 0;

    if (bytes_completed >= count)
      return 0;
    return 1;
  }
};

ssize_t Fs::read(INode_Data *inode_data, char buf[], size_t count,
                 size_t offset) {
  int err;

  if (offset >= inode_data->metadata.size)
    return 0;

  u_int64_t start_block_index = offset / IO_BLOCK_SIZE;
  size_t internal_offset = offset - (start_block_index * IO_BLOCK_SIZE);

  ReadDatablockOperation op = ReadDatablockOperation();
  op.offset = internal_offset;
  op.buf = buf;
  op.count = std::min(count, inode_data->metadata.size - offset);
  op.bytes_completed = 0;
  op.fs = this;

  if ((err = sweep_inode_datablocks(inode_data, start_block_index, false,
                                    &op)) != 0)
    return err;

  return op.bytes_completed;
}

ssize_t Fs::write(INode_Data *inode_data, const char buf[], size_t count,
                  size_t offset) {
  int err;

  // printf("test1\n");

  u_int64_t start_block_index = offset / IO_BLOCK_SIZE;
  size_t internal_offset = offset - (start_block_index * IO_BLOCK_SIZE);

  WriteDatablockOperation op = WriteDatablockOperation();
  op.offset = internal_offset;
  op.buf = buf;
  op.count = count;
  op.bytes_completed = 0;
  op.fs = this;

  // printf("test2\n");

  if ((err = sweep_inode_datablocks(inode_data, start_block_index, true,
                                    &op)) != 0)
    return err;

  // printf("test3\n");

  inode_data->metadata.size =
      std::max(offset + op.bytes_completed, inode_data->metadata.size);

  return op.bytes_completed;
}

int Fs::truncate(INode_Data *inode_data, size_t length) {
  int err;

  u_int64_t start_block_index = length / IO_BLOCK_SIZE;
  size_t internal_offset = length - (start_block_index * IO_BLOCK_SIZE);

  TruncateDatablockOperation op = TruncateDatablockOperation();
  op.offset = internal_offset;
  op.fs = this;

  if ((err = sweep_inode_datablocks(inode_data, start_block_index, false,
                                    &op)) < 0)
    return err;

  inode_data->metadata.size = length;

  return 0;
}

ssize_t Fs::lseek_next_data(INode_Data *inode_data, size_t offset) {
  int err;

  if (offset >= inode_data->metadata.size)
    return -1;

  u_int64_t start_block_index = offset / IO_BLOCK_SIZE;
  size_t internal_offset = offset - (start_block_index * IO_BLOCK_SIZE);

  LseekNextDataDatablockOperation op = LseekNextDataDatablockOperation();
  op.offset = internal_offset;
  op.count = inode_data->metadata.size;
  op.bytes_completed = offset;
  op.fs = this;

  if ((err = sweep_inode_datablocks(inode_data, start_block_index, false,
                                    &op)) < 0)
    return err;

  if (op.bytes_completed >= inode_data->metadata.size)
    return -1;

  return op.bytes_completed;
}

ssize_t Fs::lseek_next_hole(INode_Data *inode_data, size_t offset) {
  int err;

  if (offset >= inode_data->metadata.size)
    return -1;

  u_int64_t start_block_index = offset / IO_BLOCK_SIZE;
  size_t internal_offset = offset - (start_block_index * IO_BLOCK_SIZE);

  LseekNextHoleDatablockOperation op = LseekNextHoleDatablockOperation();
  op.offset = internal_offset;
  op.count = inode_data->metadata.size;
  op.bytes_completed = offset;
  op.fs = this;

  if ((err = sweep_inode_datablocks(inode_data, start_block_index, false,
                                    &op)) < 0)
    return err;

  if (op.bytes_completed >= inode_data->metadata.size)
    return inode_data->metadata.size;

  return op.bytes_completed;
}