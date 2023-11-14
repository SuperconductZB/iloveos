#ifndef DATABLOCK_ALLOCATOR_HPP
#define DATABLOCK_ALLOCATOR_HPP

class Fs;

class DataBlock_Allocator {
public:
  DataBlock_Allocator(Fs *fs, u_int64_t block_segment_start,
                      u_int64_t block_segment_end);

  virtual int new_datablock(u_int64_t *block_num) = 0;
  virtual int free_datablock(u_int64_t block_num) = 0;

  virtual int format() = 0;

protected:
  Fs *fs;
  u_int64_t block_segment_start, block_segment_end;
};

class DataBlock_Allocator_Bitmap : DataBlock_Allocator {
  using DataBlock_Allocator::DataBlock_Allocator;

  const int DATABLOCKS_PER_BITMAP_BLOCK = 255;

  int new_datablock(u_int64_t *block_num) override;
  int free_datablock(u_int64_t block_num) override;

  int format() override;
};

#endif