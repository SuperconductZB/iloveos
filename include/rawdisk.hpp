#ifndef RAWDISK_HPP
#define RAWDISK_HPP

#include <fcntl.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define BLOCK_SIZE 4096

class RawDisk {
public:
  u_int64_t diskSize;

  virtual int read_block(u_int64_t block_number, char *buffer) = 0;
  virtual int write_block(u_int64_t block_number, char *buffer) = 0;
};

#endif