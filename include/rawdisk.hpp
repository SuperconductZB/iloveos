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

  int fd;
  const char *dir;
  u_int64_t numSectors;
  u_int64_t diskSize;

public:
  RawDisk(const char *directory)
      : fd(-1), dir(nullptr), numSectors(0), diskSize(0) {
    dir = directory;
    /*dir = strdup("/dev/vdc");
    numSectors = 62914560;
    diskSize = 32212254720;*/

    // Open the block device (replace /dev/sdX with the actual device)
    fd = open(dir, O_RDWR); // Allow read and write
    if (fd == -1) {
      perror("Error opening device");
      exit(1);
    }

    // Use ioctl with BLKGETSIZE to get the number of sectors
    if (ioctl(fd, BLKGETSIZE64, &diskSize) == -1) {
      perror("Error getting disk size");
      close(fd);
      exit(1);
    }

    // Calculate the size in bytes
    numSectors = diskSize / 512; // Assuming a sector size of 512 bytes

    printf("====Initializing RawDisk====\n");
    printf("Number of sectors: %llu\n", numSectors);
    printf("Disk size (in bytes): %llu\n", diskSize);
  }

  ~RawDisk() {
    if (fd != -1) {
      close(fd);
    }
  }

  int read_block(u_int64_t block_number, char *buffer) {
    u_int64_t offset = block_number * BLOCK_SIZE;

    if (lseek(fd, offset, SEEK_SET) == (u_int64_t)-1) {
      perror("Error seeking to offset");
      return -1;
    }

    // TODO: this is incorrect
    ssize_t bytesRead = read(fd, buffer, BLOCK_SIZE);
    if (bytesRead < BLOCK_SIZE) {
      perror("Error reading from device");
      return -1;
    }

    return 0;
  }

  // Write a specified number of bytes at a given byte offset
  int write_block(u_int64_t block_number, char *buffer) {
    u_int64_t offset = block_number * BLOCK_SIZE;

    if (lseek(fd, offset, SEEK_SET) == (u_int64_t)-1) {
      perror("Error seeking to offset");
      return -1;
    }

    // TODO: this is incorrect
    ssize_t bytesWritten = write(fd, buffer, BLOCK_SIZE);
    if (bytesWritten < BLOCK_SIZE) {
      perror("Error writing to device");
      return -1;
    }

    return 0;
  }
};

#endif