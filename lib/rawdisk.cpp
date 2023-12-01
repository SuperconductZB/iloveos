#include "fs.hpp"

void RawDisk::print_block(u_int64_t block_number) {
  const int nums_per_line = 64;
  char buf[IO_BLOCK_SIZE];
  u_int64_t num;

  if (read_block(block_number, buf) < 0) {
    perror("Error printing datablock");
    return;
  }

  printf("\nBlock %llu:\n", block_number);
  for (int i = 0; i < IO_BLOCK_SIZE; i += sizeof(u_int64_t)) {
    num = 0;
    for (int j = 0; j < 8; j++)
      num |= ((u_int64_t)(unsigned char)buf[i + j]) << (8 * j);
    printf("%llu ", num);
    if ((i / sizeof(u_int64_t)) % nums_per_line == nums_per_line - 1)
      printf("\n");
  }
}

RealRawDisk::RealRawDisk(const char *directory)
    : fd(-1), dir(nullptr), numSectors(0) {
  dir = directory;
  diskSize = 0;
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

RealRawDisk::~RealRawDisk() {
  if (fd != -1) {
    close(fd);
  }
}

int RealRawDisk::read_block(u_int64_t block_number, char *buffer) {
  u_int64_t offset = block_number * IO_BLOCK_SIZE;

  if (lseek(fd, offset, SEEK_SET) == (u_int64_t)-1) {
    perror("Error seeking to offset");
    return -1;
  }

  // TODO: this is incorrect
  ssize_t bytesRead = read(fd, buffer, IO_BLOCK_SIZE);
  if (bytesRead < IO_BLOCK_SIZE) {
    perror("Error reading from device");
    return -1;
  }

  return 0;
}

int RealRawDisk::write_block(u_int64_t block_number, char *buffer) {
  u_int64_t offset = block_number * IO_BLOCK_SIZE;

  if (lseek(fd, offset, SEEK_SET) == (u_int64_t)-1) {
    perror("Error seeking to offset");
    return -1;
  }

  // TODO: this is incorrect
  ssize_t bytesWritten = write(fd, buffer, IO_BLOCK_SIZE);
  if (bytesWritten < IO_BLOCK_SIZE) {
    perror("Error writing to device");
    return -1;
  }

  return 0;
}

FakeRawDisk::FakeRawDisk(u_int64_t num_blocks) {
  diskSize = num_blocks * IO_BLOCK_SIZE;
  disk = new char[diskSize];
  if (disk == nullptr) {
    perror("Error allocating fake disk");
    exit(1);
  }
  printf("====Initializing FAKE RawDisk====\n");
  printf("FAKE Disk size (in bytes): %llu\n", diskSize);
  perror("!!! USING FAKE RawDisk - THIS IS FOR TESTING ONLY !!!");
}

FakeRawDisk::~FakeRawDisk() { delete[] disk; }

int FakeRawDisk::read_block(u_int64_t block_number, char *buffer) {
  u_int64_t offset = block_number * IO_BLOCK_SIZE;

  if (offset + IO_BLOCK_SIZE > diskSize) {
    perror("Error reading past fake disk size");
    return -1;
  }

  memcpy(buffer, &disk[offset], IO_BLOCK_SIZE);

  return 0;
}

int FakeRawDisk::write_block(u_int64_t block_number, char *buffer) {
  u_int64_t offset = block_number * IO_BLOCK_SIZE;

  printf("fake disk write: %llu %llu %llu\n", block_number, offset, diskSize);

  if (offset + IO_BLOCK_SIZE > diskSize) {
    perror("Error writing past fake disk size");
    return -1;
  }

  memcpy(&disk[offset], buffer, IO_BLOCK_SIZE);

  return 0;
}