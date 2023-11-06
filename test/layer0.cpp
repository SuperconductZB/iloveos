#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "rawdisk.h"

int main(int argc, char *argv[]) {
    const char* d = (argc < 2) ? "/dev/vdc" : argv[1];
    
    RawDisk *H = new RawDisk(d);
    
    char *buf = "iloveosdfjlseirfnerig";
    char readBuffer[512] = {0};  // Initialize to zeros

    //printf("dir %s, numSectors %lld, diskSize %lld \n", H->dir, H->numSectors, H->diskSize);

    //use number to substitute H->getnumSector(), getnumSectors() are not yest implemented
    for(off_t i = 0; i < 10; i++) {
        H->rawdisk_write(i*512, buf, sizeof(buf));//Change write_API
    }
    //use number to substitute H->getnumSector(), getnumSectors() are not yest implemented
    for(off_t i = 0; i < 10; i++) {
        H->rawdisk_read(i*512, readBuffer, sizeof(readBuffer));//Change read_API
        assert(strncmp(readBuffer, buf, strlen(buf)) == 0);
    }

    delete H;  // Delete the RawDisk object

    return 0;
}
