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
    off_t block_number;

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
        block_number = blockNumber;
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

    void inode_save(){
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
        rawdisk_write(block_number, buffer);
    }

    off_t datablock_allocate_in_list(){
        //find a free data block
        off_t freeListHead = SuperBlock::getFreeListHead();
        /*
        1. initialization
        2. data block starting position
        3. r/w between storage and rawdisk to maintain 
        */
        char buffer[512] = {0};
        off_t freeBlockNum = 0;
        rawdisk_read(freeListHead, buffer);
        for (int i = 8; i < 264; i++){
            if(buffer[i] != 255){
                for (int j = 0; j < 8; j++){
                    if ((buffer[i]&(1<<j)) == 0){
                        buffer[i] |= (1<<j);
                        break;
                    }
                }
                freeBlockNum = freeListHead + (i-8)*8 + j + 1;
            }
        }
        bool notFull = false;
        for (int i = 8; i < 264; i++){
            if((i < 263 && buffer[i] != 255) || (i == 263 && buffer[i] != 127)){
                notFull = true;
                break;
            }
        }
        if (!notFull){
            off_t next_header = 0;
            for (int j = 0; j < 8; j++)
                next_header = next_header | (((off_t)buffer[j])<<(8*j));
            writeFreeListHead(next_header);
        }
        return freeBlockNum;
    }

    // allowcate 1 datablock and add to the end of the file
    off_t datablock_allocate(){
        //do we need to check dynamic?

        //add the data block to blocks, single, double, triple
        off_t freeBlockNum = datablock_allocate_in_list();
        bool inBlocks = false;
        for (int i = 0; i < 48; i++)
            if(block[i] == 0){
                inBlocks = true;
                block[i] = freeBlockNum;
                break;
            }
        if(!inBlocks){
            if (single_indirect == 0){
                single_indirect = datablock_allocate_in_list();
            }
            int inSingle = false;
            char buffer[512] = {0};
            rawdisk_read(single_indirect, buffer);
            for (int i = 0; i < 512; i++)
                if(buffer[i] == 0){
                    inSingle = true;
                    buffer[i] = freeBlockNum;
                    break;
                }
            if(i < 512){
                rawdisk_write(single_indirect, buffer);
            }
            else{
                if (double_indirect == 0){
                    double_indirect = datablock_allocate_in_list();
                }
                int inDouble = false;
                rawdisk_read(double_indirect, buffer);
                // w.t.f is this
            }
        }
        da
        //return the block number
        inode_save();
        return freeBlockNum;
    }

    // deallocate 1 datablock from the end of the file
    void datablock_deallocate(){

    }
}

class INodeOperation{
// free list head is at super block (0): first 8 bytes

public:
    //initialization of the rawdisk
    void initialize(){
        SuperBlock::writeFreeListHead(524288); // maximum inode number
        for (off_t i = 524288; i <7864320; i += 2048){
            char buffer[512] = {0};
            off_t t = i + 2048;
            if (t < 7864320){
                for (int j = 0; j < 8; j++){
                    buffer[j] = t & (((off_t)1<<(8*j))-1);
                    t >>= 8;
                }
            }
            rawdisk_write(i, buffer);
        }
    }

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