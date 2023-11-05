#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "rawdisk.h"

int main(int argc, char *argv[]) {
    char *d = NULL;
    if(argc < 2){
        d = strdup("/dev/vdc");
    }else{
        d = argv[1];
    }
    
    RawDisk *H = new RawDisk(d);
    
    char *buf = "iloveosdfjlseirfnerig";
    char readBuffer[512] = {0};  // Initialize to zeros

    //printf("dir %s, numSectors %lld, diskSize %lld \n", H->dir, H->numSectors, H->diskSize);

    //use number to substitute H->getnumSector(), getnumSectors() are not yest implemented
    for(off_t i = 0; i < 10; i++) {
        H->rawdisk_write(i, buf);
    }
    //use number to substitute H->getnumSector(), getnumSectors() are not yest implemented
    for(off_t i = 0; i < 10; i++) {
        H->rawdisk_read(i, readBuffer);
        assert(strncmp(readBuffer, buf, strlen(buf)) == 0);
    }

    delete H;  // Delete the RawDisk object

    return 0;
}
