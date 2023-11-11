#include "rawdisk.h"
/*****************************************************
30GB Disk low-level operation and data structure: spuerblock, inode, and buffer cache
512 bytes sector for 1 block, 62914560 block(sector)
4K bytes sector for 1 block,   7864320 block(sector)

one inode equipped with one 512 bytes block

*****************************************************/
#define SECTOR_SIZE 512
#define IO_BLOCK_SIZE 4096
#define MAX_INODE 524288
#define MAX_BLOCKNUM MAX_INODE*2 //62914560

class SuperBlock{

public:
    SuperBlock(const char *directory){

    }
    ~SuperBlock(){

    }
    static u_int64_t getFreeListHead(RawDisk &disk){
        char buffer[8] = {0};
        disk.rawdisk_read(0, buffer, sizeof(buffer));
        u_int64_t t = 0;
        for (int j = 0; j < 8; j++)
            t = t | (((u_int64_t)(unsigned char)buffer[j])<<(8*j));
        return t;
    }

    static void writeFreeListHead(RawDisk &disk, u_int64_t t){
        char buffer[8] = {0};
        for (int j = 0; j < 8; j++){
            buffer[j] = (t >> (8 * j)) & 0xFF;
        }
        disk.rawdisk_write(0, buffer, sizeof(buffer));
    }

    static u_int64_t getFreeINodeHead(RawDisk &disk){
        char buffer[8] = {0};
        disk.rawdisk_read(8, buffer, sizeof(buffer));
        u_int64_t t = 0;
        for (int j = 0; j < 8; j++)
            t = t | (((u_int64_t)(unsigned char)buffer[j])<<(8*j));
        return t;
    }

    static void writeFreeINodeHead(RawDisk &disk, u_int64_t t){
        char buffer[8] = {0};
        for (int j = 0; j < 8; j++){
            buffer[j] = (t >> (8 * j)) & 0xFF;
        }
        disk.rawdisk_write(8, buffer, sizeof(buffer));
    }
};

class INode{
    // direct datablocks 
    u_int64_t blocks[48];
    // indirect address
    u_int64_t single_indirect, double_indirect, triple_indirect;
    // other 

    u_int64_t uid;
    u_int64_t gid;
    u_int64_t permissions;
    u_int64_t size;
    u_int64_t block_number;

public:
    void read_get_byte(u_int64_t &t, int &current_pos, char *buffer){
        t = 0;
        for (int j = 0; j < 8; j++)
            t = t | (((u_int64_t)(unsigned char)buffer[j+current_pos])<<(8*j));
        current_pos += 8;
    }

    static u_int64_t read_byte_at(int current_pos, char *buffer){
        u_int64_t t = 0;
        for (int j = 0; j < 8; j++)
            t = t | (((u_int64_t)(unsigned char)buffer[j+current_pos])<<(8*j));
        return t;
    }

    void inode_construct(u_int64_t blockNumber, RawDisk &disk){
        char buffer[SECTOR_SIZE] = {0};
        disk.rawdisk_read(blockNumber*SECTOR_SIZE, buffer, sizeof(buffer));
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

    void write_get_byte(u_int64_t t, int &current_pos, char *buffer){
        for (int j = 0; j < 8; j++){
            buffer[j+current_pos] = t & (((u_int64_t)1<<(8))-1);
            t >>= 8;
        }
        current_pos += 8;
    }

    static void write_byte_at(u_int64_t t, int current_pos, char *buffer){
        for (int j = 0; j < 8; j++){
            buffer[j+current_pos] = t & (((u_int64_t)1<<(8))-1);
            t >>= 8;
        }
    }

    void inode_save(RawDisk &disk){
        char buffer[SECTOR_SIZE] = {0};
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
        disk.rawdisk_write(block_number*SECTOR_SIZE, buffer, sizeof(buffer));
    }

    u_int64_t datablock_allocate_in_list(RawDisk &disk){
        //find a free data block
        u_int64_t freeListHead = SuperBlock::getFreeListHead(disk);
        /*
        1. initialization
        2. data block starting position
        3. r/w between storage and rawdisk to maintain 
        */
        char buffer[IO_BLOCK_SIZE] = {0};
        u_int64_t freeBlockNum = 0;
        disk.rawdisk_read(freeListHead, buffer, sizeof(buffer));
        for (int i = 8; i < 264; i++){
            if(buffer[i] != 255){
                int j = 0;
                for (j = 0; j < 8; j++){
                    if ((buffer[i]&(1<<j)) == 0){
                        buffer[i] |= (1<<j);
                        break;
                    }
                }
                freeBlockNum = freeListHead + ((i-8)*8 + j + 1)*IO_BLOCK_SIZE;
            }
        }
        disk.rawdisk_write(freeListHead, buffer, sizeof(buffer));
        bool notFull = false;
        for (int i = 8; i < 264; i++){
            if((i < 263 && buffer[i] != 255) || (i == 263 && buffer[i] != 127)){
                notFull = true;
                break;
            }
        }
        if (!notFull){
            u_int64_t next_header = read_byte_at(0, buffer);
            SuperBlock::writeFreeListHead(disk, next_header);
        }
        return freeBlockNum;
    }

    bool allo_single_indirect(RawDisk &disk, u_int64_t &single_i, u_int64_t freeBlockNum) {
        if (single_i == 0){
            single_i = datablock_allocate_in_list(disk);
        }
        bool inSingle = false;
        char buffer[IO_BLOCK_SIZE] = {0};
        disk.rawdisk_read(single_i, buffer, sizeof(buffer));
        for (int i = 0; i < IO_BLOCK_SIZE; i+=8){
            u_int64_t addr = read_byte_at(i, buffer);
            if(addr == 0){
                inSingle = true;
                write_byte_at(freeBlockNum, i, buffer);
                disk.rawdisk_write(single_i, buffer, sizeof(buffer));
                break;
            }
        }
        return inSingle;
    }

    bool allo_double_indirect(RawDisk &disk, u_int64_t &double_i, u_int64_t freeBlockNum) {
        if (double_i == 0){
            double_i = datablock_allocate_in_list(disk);
        }
        bool inDouble = false;
        char buffer[IO_BLOCK_SIZE] = {0};
        disk.rawdisk_read(double_i, buffer, sizeof(buffer));
        for (int i = 0; i < IO_BLOCK_SIZE; i+=8){
            u_int64_t addr = read_byte_at(i, buffer);
            bool flag = allo_single_indirect(disk, addr, freeBlockNum);
            if (flag){
                write_byte_at(addr, i, buffer);
                disk.rawdisk_write(double_i, buffer, sizeof(buffer));
                inDouble = true;
                break;
            }
        } 
        return inDouble;
    }

    bool allo_triple_indirect(RawDisk &disk, u_int64_t &triple_i, u_int64_t freeBlockNum) {
        if (triple_i == 0){
            triple_i = datablock_allocate_in_list(disk);
        }
        bool inTriple = false;
        char buffer[IO_BLOCK_SIZE] = {0};
        disk.rawdisk_read(triple_i, buffer, sizeof(buffer));
        for (int i = 0; i < IO_BLOCK_SIZE; i+=8){
            u_int64_t addr = read_byte_at(i, buffer);
            bool flag = allo_double_indirect(disk, addr, freeBlockNum);
            if (flag){
                write_byte_at(addr, i, buffer);
                disk.rawdisk_write(triple_i, buffer, sizeof(buffer));
                inTriple = true;
                break;
            }
        }
        return inTriple;
    }
    
    
    // allowcate 1 datablock and add to the end of the file
    u_int64_t datablock_allocate(RawDisk &disk){
        //do we need to check dynamic?

        //add the data block to blocks, single, double, triple
        u_int64_t freeBlockNum = datablock_allocate_in_list(disk);
        bool inBlocks = false;
        for (int i = 0; i < 48; i++)
            if(blocks[i] == 0){
                inBlocks = true;
                blocks[i] = freeBlockNum;
                break;
            }
        if(!inBlocks){
            bool inSingle = allo_single_indirect(disk, single_indirect, freeBlockNum);
            if (!inSingle){
                bool inDouble = allo_double_indirect(disk, double_indirect, freeBlockNum);
                if (!inDouble){
                    bool inTriple = allo_triple_indirect(disk, triple_indirect, freeBlockNum);
                    // wait to deal with too big files
                }
            }      
        }
        
        //return the block number
        inode_save(disk);
        return freeBlockNum;
    }

    void datablock_deallocate_in_list(u_int64_t freeBlockNum, RawDisk &disk) {
        // find the related 2048block head
        u_int64_t freeBlockHead = ((freeBlockNum/SECTOR_SIZE-MAX_INODE)/(8*2048)*(8*2048)+MAX_INODE)*SECTOR_SIZE;

        // mark it alive in its bitmap
        char buffer[IO_BLOCK_SIZE] = {0};
        bool notEmpty = false;
        disk.rawdisk_read(freeBlockHead, buffer, sizeof(buffer));
        for (int i = 8; i < 264; i++){
            if(buffer[i] != 0){
                notEmpty = true;
            }
        }
        u_int64_t inBlockPos = (freeBlockNum-freeBlockHead)/IO_BLOCK_SIZE-1;
        buffer[8+inBlockPos/8] |= (1<<(inBlockPos%8));
    
        // if its bitmap was 0, add it back to the list head
        if(!notEmpty){
            u_int64_t freeListHead = SuperBlock::getFreeListHead(disk);
            write_byte_at(freeListHead, 0, buffer);
            SuperBlock::writeFreeListHead(disk, freeBlockHead);
        }
        disk.rawdisk_write(freeBlockHead, buffer, sizeof(buffer));
    }

    u_int64_t deallo_single_indirect(RawDisk &disk, u_int64_t &single_i){
        if (single_i == 0){
            return 0;
        }
        u_int64_t freeBlockNum = 0;
        char buffer[IO_BLOCK_SIZE] = {0};
        int delpoint = -1;
        disk.rawdisk_read(single_i, buffer, sizeof(buffer));
        for (int i=4088; i >= 0; i--){
            u_int64_t addr = read_byte_at(i, buffer);
            if(addr != 0){
                freeBlockNum = addr;
                addr = 0;
                write_byte_at(addr, i, buffer);
                delpoint = i;
                break;
            }
        }
        disk.rawdisk_write(single_i, buffer, sizeof(buffer));
        u_int64_t addr = read_byte_at(0, buffer);
        if (delpoint == 0 && addr == 0){
            datablock_deallocate_in_list(single_i, disk);
            single_i = 0;
        }
        return freeBlockNum;
    }

    bool deallo_double_indirect(RawDisk &disk, u_int64_t &double_i){
        if (double_i == 0){
            return false;
        }
        u_int64_t freeBlockNum = 0;
        char buffer[IO_BLOCK_SIZE] = {0};
        int delpoint = -1;
        disk.rawdisk_read(double_i, buffer, sizeof(buffer));
        for (int i=4088; i >= 0; i-=8){
            u_int64_t addr = read_byte_at(i, buffer);
            u_int64_t inSingle = deallo_single_indirect(disk, addr);
            if (inSingle){
                freeBlockNum = inSingle;
                write_byte_at(addr, i, buffer);
                delpoint = i;
                break;
            }
        }
        disk.rawdisk_write(double_i, buffer, sizeof(buffer));
        u_int64_t addr = read_byte_at(0, buffer);
        if (delpoint == 0 && addr == 0){
            datablock_deallocate_in_list(double_i, disk);
            double_i = 0;
        }
        return freeBlockNum;
    }

    bool deallo_triple_indirect(RawDisk &disk, u_int64_t &triple_i){
        if (triple_i == 0){
            return false;
        }
        u_int64_t freeBlockNum = 0;
        char buffer[IO_BLOCK_SIZE] = {0};
        int delpoint = -1;
        disk.rawdisk_read(triple_i, buffer, sizeof(buffer));
        for (int i=4088; i >= 0; i-=8){
            u_int64_t addr = read_byte_at(i, buffer);
            u_int64_t inDouble = deallo_double_indirect(disk, addr);
            if (inDouble){
                freeBlockNum = inDouble;
                write_byte_at(addr, i, buffer);
                delpoint = i;
                break;
            }
        }
        disk.rawdisk_write(triple_i, buffer, sizeof(buffer));
        u_int64_t addr = read_byte_at(0, buffer);
        if (delpoint == 0 && addr == 0){
            datablock_deallocate_in_list(triple_i, disk);
            triple_i = 0;
        }
        return freeBlockNum;
    }

    // deallocate 1 datablock from the end of the file
    void datablock_deallocate(RawDisk &disk){
        // find the last datablock and remove it from inode (triple->direct)
        u_int64_t freeBlockNum = 0;
        freeBlockNum = deallo_triple_indirect(disk, triple_indirect);
        if(!freeBlockNum){
            freeBlockNum = deallo_double_indirect(disk, double_indirect);
            if(!freeBlockNum){
                freeBlockNum = deallo_single_indirect(disk, single_indirect);
                if(!freeBlockNum){
                    for(int i = 47; i>=0; i--)
                        if(blocks[i] != 0){
                            freeBlockNum = blocks[i];
                            blocks[i] = 0;
                            break;
                        }
                    // deal with empty
                }
            }
        }

        // add it back to freeBlocklist
        datablock_deallocate_in_list(freeBlockNum, disk);
        inode_save(disk);
    }
};

class INodeOperation{
// free list head is at super block (0): first 8 bytes

public:
    //initialization of the rawdisk
    void initialize(RawDisk &disk){
        // initialize Inode list head
        SuperBlock::writeFreeINodeHead(disk, 1);
        for (u_int64_t i = 1; i < MAX_INODE; i++){
            char buffer[SECTOR_SIZE] = {0};
            u_int64_t t = i + 1;
            if (t < MAX_INODE){
                for (int j = 0; j < 8; j++){
                    buffer[j] = (t >> (8 * j)) & 0xFF;
                }
            }
            disk.rawdisk_write(i*SECTOR_SIZE, buffer, sizeof(buffer));
        }
        SuperBlock::writeFreeListHead(disk, MAX_INODE*SECTOR_SIZE); // maximum inode number 2^19 0x80000
        //Have tested this initialize function but MAX_BLOCK too much, MAX_INODE*2 works
        for (u_int64_t i = MAX_INODE; i < MAX_BLOCKNUM-4096; i += 2048*8){
            char buffer[IO_BLOCK_SIZE] = {0};
            u_int64_t t = (i + 2048*8)*SECTOR_SIZE;
            //t is address, storing in to buffer
            for (int j = 0; j < 8; j++){
                buffer[j] = (t >> (8 * j)) & 0xFF;
            }
            disk.rawdisk_write(i*SECTOR_SIZE, buffer, sizeof(buffer));
        }
    }

    // allocate an inode from free inode list head and return the number of the inode
    // the i-th inode is in the i-th block
    u_int64_t inode_allocate(RawDisk &disk){
        u_int64_t freeINodeHead = SuperBlock::getFreeINodeHead(disk);
        char buffer[SECTOR_SIZE] = {0};
        disk.rawdisk_read(freeINodeHead*SECTOR_SIZE, buffer, sizeof(buffer));
        u_int64_t newINodeHead = INode::read_byte_at(0, buffer);
        // deal with no more INode
        SuperBlock::writeFreeINodeHead(disk, newINodeHead);
        //to do: initialize the INode on disk at freeINodeHead

        //return inode number
        return freeINodeHead;
    }

    // free the inode and add it to the free inode list head
    void inode_free(RawDisk &disk, u_int64_t INodeNumber){
        u_int64_t freeINodeHead = SuperBlock::getFreeINodeHead(disk);
        char buffer[SECTOR_SIZE] = {0};
        INode::write_byte_at(freeINodeHead, 0, buffer);
        disk.rawdisk_write(INodeNumber*SECTOR_SIZE, buffer, sizeof(buffer));
        SuperBlock::writeFreeINodeHead(disk, INodeNumber);
    }

    //ignore for now
    void inode_read(){

    }

    void inode_write(){

    }
};