#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "fs.h"
#include <inttypes.h>

int main(int argc, char *argv[]) {
    const char* d = (argc < 2) ? "/dev/vdc" : argv[1];
    
    RawDisk *H = new RawDisk(d);

    printf("test inode\n");
    INodeOperation inop;
    inop.initialize(*H);
    char buffer[8] = {0};
    //test the begining of inode 1~524287
    H->rawdisk_read((1) * SECTOR_SIZE, buffer, sizeof(buffer));
    u_int64_t t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)buffer[j]) << (8 * j);

    assert(t == 2);
    //test the number before end of inode 524286
    H->rawdisk_read((MAX_INODE - 2) * SECTOR_SIZE, buffer, sizeof(buffer));
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)buffer[j]) << (8 * j);

    assert(t == MAX_INODE - 1);
    //test the end of inode 1~524287
    H->rawdisk_read((MAX_INODE - 1) * SECTOR_SIZE, buffer, sizeof(buffer));
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)buffer[j]) << (8 * j);

    assert(t == 0);
    //test the begining of datablock
    H->rawdisk_read((MAX_INODE) * SECTOR_SIZE, buffer, sizeof(buffer));
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)buffer[j]) << (8 * j);

    assert(t == (MAX_INODE+2048*8)*SECTOR_SIZE);
    //test the end of the datablock
    H->rawdisk_read((MAX_BLOCKNUM - 2048*8) * SECTOR_SIZE, buffer, sizeof(buffer));
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)buffer[j]) << (8 * j);

    assert(t == (MAX_BLOCKNUM)*SECTOR_SIZE);

    delete H;  // Delete the RawDisk object

    return 0;
}
