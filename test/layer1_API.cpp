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
    inop.initialize(*H);//for inode initialization and datablock initialization
    char buffer[8] = {0};
    /**************************test inode Initialization***************************/
    //test the begining of inode 1th
    H->rawdisk_read((1) * SECTOR_SIZE, buffer, sizeof(buffer));
    u_int64_t t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);

    assert(t == 2);//the first 1th unused inode will store the next unused inode 2th 
    //test the number before end of inode 524286th
    H->rawdisk_read((MAX_INODE - 2) * SECTOR_SIZE, buffer, sizeof(buffer));
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);
    
    assert(t == MAX_INODE - 1);//store the maximun th inode
    //test the end of inode 524287th
    H->rawdisk_read((MAX_INODE - 1) * SECTOR_SIZE, buffer, sizeof(buffer));
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);

    assert(t == 0);//the end of inode(524287th inode) do not have the next inode address
    /**************************test datablock Initialization***************************/
    //we separate 2048 4kB I/O block(1+2047) as a group and the first I/O block will manage the following 2047 I/O block usage.
    //the first 8 bytes(0~7) in first I/O block store the address of next first I/O block, the following 256(8~263) bytes record 2047 I/O block usage.
    //test the begining of free datablock
    H->rawdisk_read((MAX_INODE) * SECTOR_SIZE, buffer, sizeof(buffer));//the begining of free datablock will store from address (MAX_INODE) * SECTOR_SIZE
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);

    assert(t == (MAX_INODE+2048*8)*SECTOR_SIZE);//the first 8 bytes of 4k I/O block will store the next address(after 2048*4k I/O block)
    //test the end of the datablock
    H->rawdisk_read((MAX_BLOCKNUM - 2048*8) * SECTOR_SIZE, buffer, sizeof(buffer));
    t = 0;
    for (int j = 0; j < 8; j++) 
        t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);

    assert(t == (MAX_BLOCKNUM)*SECTOR_SIZE);

    /***************************test inode de/allocation**********************************/
    //when requesting an inode, the inode_allocation will give you the inode number, we use inode_list to store the sequence allocate inode
    //arrary version
    int inode_list[20] = {0};
    int record_free[10] = {0};//should do a pop up structure to record the free inode 
    int rec = 9;
    //printf("Allocate 20 inode num:{");
    for(int i=0;i<20;i++){
        inode_list[i] = inop.inode_allocate(*H);
        assert(inode_list[i] == i+1);
        //printf(" %d", inode_list[i]);
    }
    //printf("}\n");
    for(int i=10;i<20;i++){
        inop.inode_free(*H,inode_list[i]);//free the 10 element from inode_list[10]
        record_free[i-10] = inode_list[i];
    }
    //allocate again the last 10
    printf("Allocate again: inode num:{");
    for(int i=10;i<20;i++){
        inode_list[i] = inop.inode_allocate(*H);
        //printf("inode %d, rec_f %d\n,", inode_list[i],record_free[rec]);
        assert(inode_list[i] == record_free[rec]);
        rec--;
    }
    printf("}\n");
    /***************************test direct blocks[48] de/allocation**********************************/
    //after free the datablock, the program will find the first smallest address of datablock to give to the inode
    //should test random resize each node, but should use datablock_free data structure to record
    INode inode_inside[10];
    u_int64_t rec_datablock_free[10][3] = {0};//array version
    for(int i=0;i<10;i++){
        inode_inside[i].inode_construct(inode_list[i],*H);
        //printf("%dth data block starting addres: ", i);
        for(int j=0;j<6;j++){
            inode_inside[i].datablock_allocate(*H);
            //printf("%d," ,inode_inside[i].datablock_allocate(*H));
        }
        //printf("\n");
    }
    for(int i=0;i<10;i++){
        //printf("%dth data block free addres: ", i);
        for(int j = 2;j >=0;j--){
            rec_datablock_free[i][j] = inode_inside[i].datablock_deallocate(*H);
            //printf("", rec_datablock_free[i][j]);
        }
        //printf("\n");
    }

    for(int i=0;i<10;i++){
        //printf("%dth data block allocate again addres: ", i);
        for(int j=0;j<3;j++){
            assert(inode_inside[i].datablock_allocate(*H) == rec_datablock_free[i][j]);
            //printf("%d," ,inode_inside[i].datablock_allocate(*H));
        }
        //printf("\n");
    }

    //printf("}\n");
    delete H;  // Delete the RawDisk object

    return 0;
}
