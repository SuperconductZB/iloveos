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
        t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);

    assert(t == 2);
    //test the number before end of inode 524286
    H->rawdisk_read((MAX_INODE - 2) * SECTOR_SIZE, buffer, sizeof(buffer));
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);
    
    fprintf(stderr,"[t %llu,%d]\n",t,__LINE__);
    assert(t == MAX_INODE - 1);
    //test the end of inode 1~524287
    H->rawdisk_read((MAX_INODE - 1) * SECTOR_SIZE, buffer, sizeof(buffer));
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);

    assert(t == 0);
    //test the begining of datablock
    H->rawdisk_read((MAX_INODE) * SECTOR_SIZE, buffer, sizeof(buffer));
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);

    assert(t == (MAX_INODE+2048*8)*SECTOR_SIZE);
    //test the end of the datablock
    H->rawdisk_read((MAX_BLOCKNUM - 2048*8) * SECTOR_SIZE, buffer, sizeof(buffer));
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)(char)buffer[j]) << (8 * j);

    assert(t == (MAX_BLOCKNUM)*SECTOR_SIZE);
    //initialize
    int inode_list[20] = {0};
    printf("Allocate 20 inode num:{");
    for(int i=0;i<20;i++){
        inode_list[i] = inop.inode_allocate(*H);
        printf(" %d", inode_list[i]);
    }
    printf("}\n");
    //free the last 10
    printf("Free: inode num:{");
    for(int i=10;i<20;i++){
        inop.inode_free(*H,inode_list[i]);
        printf(" %d", inode_list[i]);
    }
    printf("}\n");
    //allocate again the last 10
    printf("Allocate again: inode num:{");
    for(int i=10;i<20;i++){
        inode_list[i] = inop.inode_allocate(*H);
        printf(" %d", inode_list[i]);
    }
    printf("}\n");

    printf("The status 20 inode num:{");
    for(int i=0;i<20;i++){
        printf(" %d", inode_list[i]);
    }
    printf("}\n");
    delete H;  // Delete the RawDisk object

    return 0;
}
