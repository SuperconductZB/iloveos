#include "fs.hpp"

DataBlock_Allocator::DataBlock_Allocator(Fs *fs, u_int64_t block_segment_start,
                                         u_int64_t block_segment_end) {
  fs = fs;
  block_segment_start = block_segment_start;
  block_segment_end = block_segment_end;
}

class BitmapBlock_Data {
  char buf[BLOCK_SIZE];

  u_int64_t get_next_node() {
    u_int64_t block_num;
    read_u64(&block_num, buf) return block_num;
  }
  void set_next_node(u_int64_t block_num) { write_u64(block_num, buf); }

  u_int64_t claim_relative_block() {
    // This type u_int64_t is important
    u_int64_t *data = &buf[8];
    u_int64_t relative_block_num = 0;

    for (size_t i = 0; i < (BLOCK_SIZE / 8) - 1; ++i)
      if (data[i] != (~0))
        for (size_t j = 0; j < 64; ++j)
          if (data[i] & (1 << j)) {
            data[i] |= (1 << j);
            return (i * 64) + j + 1;
          }

    perror("Error claiming block from bitmap");
    return 0;
  }
  void release_relative_block(u_int64_t relative_block_num) {
    relative_block_num -= 1;
    size_t index = (relative_block_num / 8) + 8;
    int offset = relative_block_num % 8;
    buf[index] &= ~(1 << offset);
  }
}

int DataBlock_Allocator_Bitmap::new_datablock(u_int64_t *block_num) {
  int err;
  BitmapBlock_Data bitmap = BitmapBlock_Data();
  u_int64_t bitmap_block_num = fs->superblock.free_list_head;

  if ((err = disk->read_block(bitmap_block_num, bitmap.buf)) < 0)
    return err;

  u_int64_t relative_block_num = bitmap.claim_relative_block();

  if (relative_block_num == DATABLOCKS_PER_BITMAP_BLOCK) {
    fs->superblock.free_list_head = bitmap.get_next_node();
    if ((err = fs->store_superblock()) < 0)
      return err;
  }

  bitmap.set_next_node(0);

  if ((err = disk->write_block(bitmap_block_num, bitmap.buf)) < 0)
    return err;

  (*block_num) = relative_block_num + bitmap_block_num;
  return 0;
}

int DataBlock_Allocator_Bitmap::free_datablock(u_int64_t block_num) {
  return -1;

  // sticing almost full bitmaps back at start of freelist os slow
  // also potentially like 256 times slower
}

int DataBlock_Allocator_Bitmap::format() { return -1; }