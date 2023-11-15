#include "rawdisk.hpp"

class RealRawDisk : RawDisk {
public:
  int fd;
  const char *dir;
  u_int64_t numSectors;

  RealRawDisk(const char *directory)
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

  ~RealRawDisk() {
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

class FakeRawDisk : RawDisk {
public:
  char *disk;

  FakeRawDisk(u_int64_t num_blocks) {
    disksize = num_blocks * BLOCK_SIZE;
    disk = new char[disksize];
    if (disk == nullptr) {
      perror("Error allocating fake disk");
      exit(1);
    }
    printf("====Initializing FAKE RawDisk====\n");
    printf("FAKE Disk size (in bytes): %llu\n", diskSize);
    perror("!!! USING FAKE RawDisk - THIS IS FOR TESTING ONLY !!!");
  }

  ~FakeRawDisk() { delete[] disk; }

  int read_block(u_int64_t block_number, char *buffer) {
    u_int64_t offset = block_number * BLOCK_SIZE;

    if (offset + BLOCK_SIZE > diskSize) {
      perror("Error reading past fake disk size");
      return -1;
    }

    memcpy(buffer, &disk[offset], BLOCK_SIZE);

    return 0;
  }

  int write_block(u_int64_t block_number, char *buffer) {
    u_int64_t offset = block_number * BLOCK_SIZE;

    if (offset + BLOCK_SIZE > diskSize) {
      perror("Error writing past fake disk size");
      return -1;
    }

    memcpy(&disk[offset], buffer, BLOCK_SIZE);

    return 0;
  }
};