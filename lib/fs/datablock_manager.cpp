#include "fs.hpp"

DataBlock_Manager::DataBlock_Manager(Fs *fs, u_int64_t block_segment_start,
                                     u_int64_t block_segment_end)
    : fs(fs), block_segment_start(block_segment_start),
      block_segment_end(block_segment_end) {}

class BitmapBlock_Data {
public:
  char buf[IO_BLOCK_SIZE];
  u_int64_t datablocks_per_bitmap;

  BitmapBlock_Data(u_int64_t datablocks_per_bitmap_)
      : datablocks_per_bitmap(datablocks_per_bitmap_) {}

  u_int64_t get_next_node() {
    u_int64_t block_num;
    read_u64(&block_num, buf);
    return block_num;
  }
  void set_next_node(u_int64_t block_num) { write_u64(block_num, buf); }

  u_int64_t find_unfilled() {
    const char *data = &buf[8];
    u_int64_t i = 0;

    for (; i < datablocks_per_bitmap; ++i)
      if ((data[i / 8] & (1 << (i % 8))) == 0)
        return i + 1;

    return 0;
  }
  u_int64_t claim_relative_block() {
    u_int64_t unfilled = find_unfilled();
    if (unfilled)
      buf[((unfilled - 1) / 8) + 8] |= (1 << ((unfilled - 1) % 8));
    return unfilled;
  }

  void release_relative_block(u_int64_t relative_block_num) {
    relative_block_num -= 1;
    size_t index = (relative_block_num / 8) + 8;
    int offset = relative_block_num % 8;
    buf[index] &= ~(1 << offset);
  }
};

int DataBlock_Manager_Bitmap::new_datablock(u_int64_t *block_num) {
  int err;
  BitmapBlock_Data bitmap = BitmapBlock_Data(DATABLOCKS_PER_BITMAP_BLOCK);
  u_int64_t bitmap_block_num = fs->superblock.free_list_head;
  char zero_buf[IO_BLOCK_SIZE] = {0};

  if (bitmap_block_num < block_segment_start ||
      bitmap_block_num >= block_segment_end)
    return -1;

  if ((err = fs->disk->read_block(bitmap_block_num, bitmap.buf)) < 0)
    return err;

  // if (bitmap.get_next_node() == fs->superblock.free_list_head)
  //   printf("WARNING: ON LAST BITMAP "
  //          "BLOCK!\n");

  u_int64_t relative_block_num = bitmap.claim_relative_block();

  if (relative_block_num == 0)
    return -1;

  u_int64_t block_num_ = relative_block_num + bitmap_block_num;

  // NOTE: this could be removed for speed
  if ((err = fs->disk->write_block(block_num_, zero_buf)) < 0)
    return err;

  // Could be optimized
  if (bitmap.find_unfilled() == 0) {
    if ((err = fs->save_free_list_head(bitmap.get_next_node())) < 0)
      return err;
    bitmap.set_next_node(0);
  }

  if ((err = fs->disk->write_block(bitmap_block_num, bitmap.buf)) < 0)
    return err;

  (*block_num) = block_num_;
  return 0;
}

int DataBlock_Manager_Bitmap::free_datablock(u_int64_t block_num) {
  int err;
  BitmapBlock_Data bitmap = BitmapBlock_Data(DATABLOCKS_PER_BITMAP_BLOCK);
  const u_int64_t bitmap_region_size = DATABLOCKS_PER_BITMAP_BLOCK + 1;
  bool update_freelist = false;

  u_int64_t bitmap_block_num =
      (((block_num - block_segment_start) / bitmap_region_size) *
       bitmap_region_size) +
      block_segment_start;

  if ((err = fs->disk->read_block(bitmap_block_num, bitmap.buf)) < 0)
    return err;

  if (bitmap.find_unfilled() == 0) {
    update_freelist = true;
    bitmap.set_next_node(fs->superblock.free_list_head);
  }

  bitmap.release_relative_block(block_num - bitmap_block_num);

  if ((err = fs->disk->write_block(bitmap_block_num, bitmap.buf)) < 0)
    return err;

  if (update_freelist)
    if ((err = fs->save_free_list_head(bitmap_block_num)) < 0)
      return err;

  return 0;

  // placing almost full bitmaps back at start of freelist is slow
  // potentially like 256 times slower throughput
}

int DataBlock_Manager_Bitmap::format() {
  const u_int64_t bitmap_region_size = DATABLOCKS_PER_BITMAP_BLOCK + 1;
  char buf[IO_BLOCK_SIZE] = {0};
  int err;
  u_int64_t i = block_segment_start;
  write_u64(i, buf);
  for (; i <= block_segment_end - (2 * bitmap_region_size);
       i += bitmap_region_size) {
    write_u64(i + bitmap_region_size, buf);
    if ((err = fs->disk->write_block(i, buf)) < 0)
      return err;
  }
  if ((err = fs->disk->write_block(i, buf)) < 0)
    return err;
  if ((err = fs->save_free_list_head(block_segment_start)) < 0)
    return err;
  return 0;
}