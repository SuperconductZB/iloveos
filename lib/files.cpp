//#include "fuse.h" add this when layer3
#include "files.h"
#include <string.h>
#include <sstream>

/*********************************************************************
                Directory Entry definition and function


*********************************************************************/
struct DirectoryEntry {
    u_int64_t inode_number;
    char file_name[56];
    void serialize(char* buffer) {
        u_int64_t t = inode_number;
        for (int j = 0; j < 8; j++){
            buffer[j] = t & (((u_int64_t)1<<(8))-1);
            t >>= 8;
        }
        strcpy(buffer+8, file_name);
    }
    void deserialize(char* buffer) {
        inode_number = 0;
        for (int j = 0; j < 8; j++)
            inode_number = inode_number | (((u_int64_t)(unsigned char)buffer[j])<<(8*j));
        strcpy(file_name, buffer+8);
    }
};
//Directory collection
typedef struct {
    //DirectoryEntry entries[MAX_ENTRIES]; with tree structure
    unsigned int num_entries;
} Directory;
/*
 *	fishcl_add_entry()
 *
 * adds a file entry to the specified directory, using the same
 * semantics as fishcl_find_entry(). It returns NULL if it failed.
 *
 */
static int fishcl_add_entry(Directory *dir, unsigned int inode_number, const char *name){

}

FilesOperation::FilesOperation(RawDisk& disk_): disk(disk_) {
    inop.initialize(disk);
}

u_int64_t index_to_offset(const INode& inode, RawDisk& disk, u_int64_t index) {
    if (index < 48) {
        return inode.blocks[index];
    } else if (index < 48 + 512){
        char indirect_buffer[IO_BLOCK_SIZE] = {0};
        disk.rawdisk_read(inode.single_indirect, indirect_buffer, IO_BLOCK_SIZE);
        return INode::read_byte_at(8*(index-48), indirect_buffer);
    } else if (index < 48 + 512 + 512*512) {
        char indirect_buffer[IO_BLOCK_SIZE] = {0};
        disk.rawdisk_read(inode.double_indirect, indirect_buffer, IO_BLOCK_SIZE);
        u_int64_t offset = INode::read_byte_at(8*((index-48-512)/512), indirect_buffer);
        disk.rawdisk_read(offset,indirect_buffer, IO_BLOCK_SIZE);
        return INode::read_byte_at(8*((index-48-512)%512), indirect_buffer);
    } else if (index < 48 + 512 + 512*512 + 512*512*512){
        char indirect_buffer[IO_BLOCK_SIZE] = {0};
        disk.rawdisk_read(inode.triple_indirect, indirect_buffer, IO_BLOCK_SIZE);
        u_int64_t offset = INode::read_byte_at(8*((index-48-512-512*512)/(512*512)), indirect_buffer);
        disk.rawdisk_read(offset,indirect_buffer, IO_BLOCK_SIZE);
        offset = INode::read_byte_at(8*(((index-48-512-512*512)%(512*512))/512), indirect_buffer);
        disk.rawdisk_read(offset,indirect_buffer, IO_BLOCK_SIZE);
        return INode::read_byte_at(8*((index-48-512-512*512)%512), indirect_buffer);
    } else {
        printf("index out of range, tried to access index %llu, max index %llu\n", index, 48+512+512*512+512*512*512);
        return -1;
    }
}

int FilesOperation::read_datablock(const INode& inode, u_int64_t index, char* buffer) {
    if (index >= inode.size) {
        printf("Read datablock out of range, inode number %llu", inode.block_number);
        return -1;
    }
    u_int64_t read_offset = index_to_offset(inode, disk, index);
    if (read_offset == (u_int64_t)(-1)) {
        return -1;
    }
    return disk.rawdisk_read(read_offset, buffer, IO_BLOCK_SIZE);
}

int FilesOperation::write_datablock(INode& inode, u_int64_t index, const char* buffer) {
    while (index >= inode.size) {
        u_int64_t ret = inode.datablock_allocate(disk);
        inode.size += 1;
    }
    u_int64_t write_offset = index_to_offset(inode, disk, index);
    if (write_offset == (u_int64_t)(-1)) {
        return -1;
    }
    return disk.rawdisk_write(write_offset, buffer, IO_BLOCK_SIZE);
}

INode* FilesOperation::new_inode(u_int64_t inode_number, u_int64_t permissions) {
    // zero out disk space of inode, because in memory inode is uninitialized by default
    char buffer[SECTOR_SIZE] = {0};
    disk.rawdisk_write(inode_number*SECTOR_SIZE, buffer, sizeof(buffer));
    INode *inode = new INode;
    inode->inode_construct(inode_number, disk);
    inode->block_number = inode_number;
    inode->permissions = permissions;
    inode->inode_save(disk);

    return inode;
}

void FilesOperation::create_dot_dotdot(INode* inode, u_int64_t parent_inode_number) {
    if(inode->size != 0) {
        printf("Error: create_dot_dotdot should only be called on new inode for directory\n");
    }
    char buffer[IO_BLOCK_SIZE] = {0};
    DirectoryEntry dot;
    dot.inode_number = inode->block_number;
    strcpy(dot.file_name, ".");
    dot.serialize(buffer);
    DirectoryEntry dotdot;
    dotdot.inode_number = parent_inode_number;
    strcpy(dotdot.file_name, "..");
    dotdot.serialize(buffer+64);
    int ret = write_datablock(*inode, 0, buffer);
    inode->inode_save(disk);
}

void FilesOperation::initialize_rootinode() {
    // this method must be called explicitly right after initializion
    root_inode = inop.inode_allocate(disk);
    printf("Info: root inode number: %llu\n", root_inode);
    INode *get_inode = new_inode(root_inode, S_IFDIR);
    create_dot_dotdot(get_inode, root_inode);
    delete get_inode;
}

void FilesOperation::printDirectory(u_int64_t inode_number) {
    INode inode;
    inode.inode_construct(inode_number, disk);
    char buffer[IO_BLOCK_SIZE] = {0};
    read_datablock(inode, 0, buffer);
    DirectoryEntry ent;
    for(int i=0;i<=IO_BLOCK_SIZE-64;i+=64){
        ent.deserialize(buffer+i);
        printf("%d(%s)\t", i, ent.file_name);
    }
    printf("\n");
}

u_int64_t FilesOperation::create_new_inode(u_int64_t parent_inode_number, const char* name, mode_t mode) {
    // trys to create a file under parent directory
    if (strlen(name)>=56) {
        perror("Name too long, cannot create file or directory");
        return -1;
    }
    INode inode;
    inode.inode_construct(parent_inode_number, disk);
    if ((inode.permissions & S_IFMT) != S_IFDIR) {
        printf("Parent Inode is not a directory\n");
        return -1;
    }
    char buffer[IO_BLOCK_SIZE] = {0};
    if (inode.size > 0) read_datablock(inode, 0, buffer);
    
    // do create inode
    u_int64_t new_inode_number = 0;
    DirectoryEntry ent;
    for(int i=0;i<=IO_BLOCK_SIZE-64;i+=64){
        ent.deserialize(buffer+i);
        if (ent.inode_number == 0) {
            new_inode_number = inop.inode_allocate(disk);
            ent.inode_number = new_inode_number;
            strcpy(ent.file_name, name);
            ent.serialize(buffer+i);
            break;
        }
    }
    if (new_inode_number == 0) {
        perror("Failed to create file in directory: First datablock full");
        return -1;
    } else {
        write_datablock(inode, 0, buffer);
    }
    inode.inode_save(disk);

    // initialize new file
    INode *get_inode = new_inode(new_inode_number, mode);
    if ((get_inode->permissions & S_IFMT) == S_IFDIR) {
        create_dot_dotdot(get_inode, parent_inode_number);
    }
    delete get_inode;
    return new_inode_number;
}

u_int64_t FilesOperation::namei(const char* path) {
    // returns the inode number corresponding to path
    u_int64_t current_inode = root_inode;
    std::string current_dirname;
    std::istringstream pathStream(path);
	std::string new_name;
    std::getline(pathStream, new_name, '/');
	if(!new_name.empty()){
		printf("namei: path should start with /\n");
		return -1;
	}
	while (std::getline(pathStream, new_name, '/')) {
		INode inode;
        inode.inode_construct(current_inode, disk);
        if ((inode.permissions & S_IFMT) != S_IFDIR || inode.size == 0) {
            printf("namei: %s is not a non-empty directory\n", current_dirname.c_str());
            return -1;
        }
        char buffer[IO_BLOCK_SIZE] = {0};
        read_datablock(inode, 0, buffer);
        u_int64_t new_inode_number = 0;
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-64;i+=64){
            ent.deserialize(buffer+i);
            if (strcmp(ent.file_name, new_name.c_str()) == 0) {
                new_inode_number = ent.inode_number;
                break;
            }
        }
        if (!new_inode_number) {
            printf("namei: no name matching %s under directory %s\n", new_name.c_str(), current_dirname.c_str());
            return -1;
        }
        current_inode = new_inode_number;
        current_dirname = new_name;
	}
    return current_inode;
    // path = "/" should return root_inode
    // path = "/foo.txt" should return inode for foo.txt
    // path = "/mydir" should return inode for mydir
    // path = "/nonemptydir/foo" should return inode for foo
    // path = "/notnonemptydir/foo" should raise error
}

/**/
u_int64_t FilesOperation::fischl_mkdir(const char* path, mode_t mode) {
    //check path
    char *pathdup = strdup(path);
    char *lastSlash = strrchr(pathdup, '/');
    *lastSlash = '\0'; // Split the string into parent path and new directory name; <parent path>\0<direcotry name>
    char *newDirname = lastSlash+1; //\0<direcotry name>, get from <direcotry name>
    char *ParentPath = pathdup;//pathdup are separated by pathdup, so it take <parent path> only

    u_int64_t parent_inode_number = namei(ParentPath);
    if (parent_inode_number == (u_int64_t)-1) {
        printf("fischl_mkdir failed because namei failed to find inode for %s\n", ParentPath);
        delete pathdup;
        return -1;
    }
    //make new inode
    u_int64_t ret = create_new_inode(parent_inode_number, newDirname, mode|S_IFDIR);//specify S_IFDIR as directory 
    delete pathdup;
    return ret;
    //after new_inode(mkfile), go to fischl_add_entry record

    
}

u_int64_t FilesOperation::fischl_mknod(const char* path, mode_t mode) {
    //check path
    char *pathdup = strdup(path);
    char *lastSlash = strrchr(pathdup, '/');
    *lastSlash = '\0'; // Split the string into parent path and new directory name; <parent path>\0<direcotry name>
    char *newFilename = lastSlash+1; //\0<direcotry name>, get from <direcotry name>
    char *ParentPath = pathdup;//pathdup are separated by pathdup, so it take <parent path> only

    u_int64_t parent_inode_number = namei(ParentPath);
    if (parent_inode_number == (u_int64_t)-1) {
        printf("fischl_mkdir failed because namei failed to find inode for %s\n", ParentPath);
        delete pathdup;
        return -1;
    }
    //make new inode
    u_int64_t ret = create_new_inode(parent_inode_number, newFilename, mode);
    delete pathdup;
    return ret;
}