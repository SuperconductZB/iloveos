#ifndef DATABLOCK_MANAGER_HPP
#define DATABLOCK_MANAGER_HPP

#include "fs_constants.hpp"

class Fs;

class DataBlock_Manager {
public:
  DataBlock_Manager(Fs *fs, u_int64_t block_segment_start,
                      u_int64_t block_segment_end);

  virtual int new_datablock(u_int64_t *block_num) = 0;
  virtual int free_datablock(u_int64_t block_num) = 0;

  virtual int format() = 0;

protected:
  Fs *fs;
  u_int64_t block_segment_start, block_segment_end;
};

class DataBlock_Manager_Bitmap : public DataBlock_Manager {
public:
  DataBlock_Manager_Bitmap(Fs *fs, u_int64_t block_segment_start,
                             u_int64_t block_segment_end)
      : DataBlock_Manager(fs, block_segment_start, block_segment_end) {}

  int new_datablock(u_int64_t *block_num) override;
  int free_datablock(u_int64_t block_num) override;

  int format() override;
};

#endif