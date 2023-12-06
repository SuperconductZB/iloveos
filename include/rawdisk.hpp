#ifndef RAWDISK_HPP
#define RAWDISK_HPP

#include "fs_constants.hpp"

class RawDisk {
public:
  u_int64_t diskSize;

  virtual int read_block(u_int64_t block_number, char *buffer) = 0;
  virtual int write_block(u_int64_t block_number, char *buffer) = 0;

  void print_block(u_int64_t block_number);
};

class RealRawDisk : public RawDisk {
public:
  int fd;
  const char *dir;
  u_int64_t numSectors;

  RealRawDisk(const char *directory, u_int64_t _diskSize = 0);
  ~RealRawDisk();

  int read_block(u_int64_t block_number, char *buffer) override;
  int write_block(u_int64_t block_number, char *buffer) override;
};

class FakeRawDisk : public RawDisk {
public:
  char *disk;

  FakeRawDisk(u_int64_t num_blocks);
  ~FakeRawDisk();

  int read_block(u_int64_t block_number, char *buffer) override;
  int write_block(u_int64_t block_number, char *buffer) override;
};

#endif