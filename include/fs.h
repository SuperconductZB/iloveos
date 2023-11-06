#include "rawdisk.h"

class SuperBlock{
public:
    static off_t getFreeListHead(){
        char buffer[512] = {0};
        rawdisk_read(0, buffer);
        off_t t = 0;
        for (int j = 0; j < 8; j++)
            t = t | (((off_t)buffer[j])<<(8*j));
        return t;
    }

    static void writeFreeListHead(){
        char buffer[512] = {0};
        for (int j = 0; j < 8; j++){
            buffer[j] = t & (((off_t)1<<(8*j))-1);
            t >>= 8;
        }
        rawdisk_write(0, buffer);
    }
};

class INode{
    // direct datablocks 
    off_t blocks[48];
    // indirect address
    off_t single_indirect, double_indirect, triple_indirect;
    // other 

    off_t uid;
    off_t gid;
    off_t permissions;
    off_t size;

public:
    void read_get_byte(off_t &t, int &current_pos, char *buffer){
        t = 0;
        for (int j = 0; j < 8; j++)
            t = t | (((off_t)buffer[j+current_pos])<<(8*j));
        current_pos += 8;
    }

    void inode_construct(off_t blockNumber){
        char buffer[512] = {0};
        rawdisk_read(blockNumber, buffer);
        int current_pos = 0;
        // initialize blocks
        for (int i = 0; i < 48; i++){
            read_get_byte(block[i], current_pos, buffer);
        }
        read_get_byte(single_indirect, current_pos, buffer);
        read_get_byte(double_indirect, current_pos, buffer);
        read_get_byte(triple_indirect, current_pos, buffer);
        read_get_byte(uid, current_pos, buffer);
        read_get_byte(gid, current_pos, buffer);
        read_get_byte(permissions, current_pos, buffer);
        read_get_byte(size, current_pos, buffer);
    }

    void write_get_byte(off_t t, int &current_pos, char *buffer){
        for (int j = 0; j < 8; j++){
            buffer[j+current_pos] = t & (((off_t)1<<(8*j))-1);
            t >>= 8;
        }
        current_pos += 8;
    }

    void inode_save(off_t blockNumber){
        char buffer[512] = {0};
        int current_pos = 0;
        for (int i = 0; i < 48; i++){
            write_get_byte(block[i], current_pos, buffer);
        }
        write_get_byte(single_indirect, current_pos, buffer);
        write_get_byte(double_indirect, current_pos, buffer);
        write_get_byte(triple_indirect, current_pos, buffer);
        write_get_byte(uid, current_pos, buffer);
        write_get_byte(gid, current_pos, buffer);
        write_get_byte(permissions, current_pos, buffer);
        write_get_byte(size, current_pos, buffer);
        rawdisk_write(bloackNumber, buffer);
    }

    // allowcate 1 datablock and add to the end of the file
    off_t datablock_allocate(){
        //do we need to check dynamic?
        
        //find a free data block
        off_t freeListHead = SuperBlock::getFreeListHead();
        for (freeListHead)

        //add the data block to blocks, single, double, triple
        
        //return the block number

    }

    // deallocate 1 datablock from the end of the file
    void datablock_deallocate(){

    }
}

class INodeOperation{
// free list head is at super block (0): first 8 bytes

public:

    // allocate an inode and return the number of the inode
    // the i-th inode is in the i-th block
    off_t inode_allocate(){
        
        //return inode number
    }

    // 
    void inode_free(off_t iNodeNumber){

    }

    //ignore for now
    void inode_read(){

    }

    void inode_write(){

    }
}