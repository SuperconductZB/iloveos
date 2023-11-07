#include "rawdisk.h"
/*****************************************************
30GB Disk low-level operation and data structure: spuerblock, inode, and buffer cache
512 bytes sector for 1 block, 62914560 block(sector)
4K bytes sector for 1 block,   7864320 block(sector)

one inode equipped with one 512 bytes block

*****************************************************/
#define MAX_INODE 524288
#define MAX_BLOCKNUM 62914560

class SuperBlock{

public:
    SuperBlock(const char *directory){

    }
    ~SuperBlock(){

    }
    static off_t getFreeListHead(RawDisk &disk){
        char buffer[512] = {0};
        disk.rawdisk_read(0, buffer, sizeof(buffer));
        off_t t = 0;
        for (int j = 0; j < 8; j++)
            t = t | (((off_t)buffer[j])<<(8*j));
        return t;
    }

    static void writeFreeListHead(RawDisk &disk, off_t t){
        char buffer[512] = {0};
        for (int j = 0; j < 8; j++){
            buffer[j] = (t >> (8 * j)) & 0xFF;
        }
        disk.rawdisk_write(0, buffer, sizeof(buffer));
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

    void inode_construct(off_t blockNumber, RawDisk &disk){
        char buffer[512] = {0};
        disk.rawdisk_read(blockNumber*512, buffer, sizeof(buffer));
        block_number = blockNumber;
        int current_pos = 0;
        // initialize blocks
        for (int i = 0; i < 48; i++){
            read_get_byte(blocks[i], current_pos, buffer);
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

    void inode_save(RawDisk &disk){
        char buffer[512] = {0};
        int current_pos = 0;
        for (int i = 0; i < 48; i++){
            write_get_byte(blocks[i], current_pos, buffer);
        }
        write_get_byte(single_indirect, current_pos, buffer);
        write_get_byte(double_indirect, current_pos, buffer);
        write_get_byte(triple_indirect, current_pos, buffer);
        write_get_byte(uid, current_pos, buffer);
        write_get_byte(gid, current_pos, buffer);
        write_get_byte(permissions, current_pos, buffer);
        write_get_byte(size, current_pos, buffer);
        disk.rawdisk_write(block_number*512, buffer, sizeof(buffer));
    }

    off_t datablock_allocate_in_list(RawDisk &disk){
        //find a free data block
        off_t freeListHead = SuperBlock::getFreeListHead(disk);
        /*
        1. initialization
        2. data block starting position
        3. r/w between storage and rawdisk to maintain 
        */
        char buffer[512] = {0};
        off_t freeBlockNum = 0;
        disk.rawdisk_read(freeListHead*512, buffer, sizeof(buffer));
        for (int i = 8; i < 264; i++){
            if(buffer[i] != 255){
                int j = 0;
                for (j = 0; j < 8; j++){
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
            SuperBlock::writeFreeListHead(disk, next_header);
        }
        return freeBlockNum;
    }

    // allowcate 1 datablock and add to the end of the file
    off_t datablock_allocate(RawDisk &disk){
        //do we need to check dynamic?

        //add the data block to blocks, single, double, triple
        off_t freeBlockNum = datablock_allocate_in_list(disk);
        bool inBlocks = false;
        for (int i = 0; i < 48; i++)
            if(blocks[i] == 0){
                inBlocks = true;
                blocks[i] = freeBlockNum;
                break;
            }
        if(!inBlocks){
            if (single_indirect == 0){
                single_indirect = datablock_allocate_in_list(disk);
            }
            int inSingle = false;
            char buffer[512] = {0};
            disk.rawdisk_read(single_indirect*512, buffer, sizeof(buffer));
            for (int i = 0; i < 512; i++){
                if(buffer[i] == 0){
                    inSingle = true;
                    buffer[i] = freeBlockNum;
                    break;
                }
                //ask which scope    
                if(i < 512){
                    disk.rawdisk_write(single_indirect*512, buffer, sizeof(buffer));
                }
                else{
                    if (double_indirect == 0){
                        double_indirect = datablock_allocate_in_list(disk);
                    }
                    int inDouble = false;
                    disk.rawdisk_read(double_indirect*512, buffer, sizeof(buffer));
                    // w.t.f is this
                }
            }
                
        }
        
        //return the block number
        inode_save(disk);
        return freeBlockNum;
    }

    // deallocate 1 datablock from the end of the file
    void datablock_deallocate(){

    }
};

class INodeOperation{
// free list head is at super block (0): first 8 bytes

public:
    //initialization of the rawdisk
    void initialize(RawDisk &disk){
        SuperBlock::writeFreeListHead(disk, MAX_INODE); // maximum inode number 2^19 0x80000
        //Have tested this initialize function but MAX_BLOCK too much, MAX_INODE*2 works
        for (off_t i = MAX_INODE; i < MAX_BLOCKNUM; i += 2048){
            char buffer[512] = {0};
            off_t t = i + 2048;
            if (t < MAX_BLOCKNUM){
                for (int j = 0; j < 8; j++){
                    buffer[j] = (t >> (8 * j)) & 0xFF;
                }
            }
            disk.rawdisk_write(i*512, buffer, sizeof(buffer));
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
};