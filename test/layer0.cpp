#include "rawdisk.hpp"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  const char *d = (argc < 2) ? "/dev/vdc" : argv[1];

  RawDisk *H = new RealRawDisk(d);

  char *buf = "iloveosdfjlseirfnerig";
  char readBuffer[IO_BLOCK_SIZE] = {0}; // Initialize to zeros

  // printf("dir %s, numSectors %lld, diskSize %lld \n", H->dir, H->numSectors,
  // H->diskSize);

  // use number to substitute H->getnumSector(), getnumSectors() are not yest
  // implemented
  for (u_int64_t i = 0; i < 10; i++) {
    H->write_block(i, buf); // Change write_API
  }
  // use number to substitute H->getnumSector(), getnumSectors() are not yest
  // implemented
  for (u_int64_t i = 0; i < 10; i++) {
    H->read_block(i, readBuffer); // Change read_API
    assert(strncmp(readBuffer, buf, strlen(buf)) == 0);
  }

  delete H; // Delete the RawDisk object

  return 0;
}
