#include "files.h"
#include <string.h>
#include <sstream>

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

FilesOperation::FilesOperation(RawDisk& disk_): disk(disk_) {
    disk = disk_;
    inop.initialize(disk_);
}

int FilesOperation::read_datablock(INode& inode, u_int64_t index, char* buffer) {
    if (index >= inode.size) {
        printf("Read datablock out of range, inode number %llu", inode.block_number);
        return -1;
    }
    if (index < 48) {
        return disk.rawdisk_read(inode.blocks[index], buffer, IO_BLOCK_SIZE);
    } else {
        perror("Read indirect datablocks not implemented yet");
        return -1;
    }
}

int FilesOperation::write_datablock(INode& inode, u_int64_t index, char* buffer) {
    while (index >= inode.size) {
        u_int64_t ret = inode.datablock_allocate(disk);
        inode.size += 1;
    }
    if (index < 48) {
        return disk.rawdisk_write(inode.blocks[index], buffer, IO_BLOCK_SIZE);
    } else {
        perror("Write indirect datablocks not implemented yet");
        return -1;
    }
}

void FilesOperation::init_inode(u_int64_t inode_number, u_int64_t permissions) {
    // zero out disk space of inode, because in memory inode is uninitialized by default
    char buffer[SECTOR_SIZE] = {0};
    disk.rawdisk_write(inode_number*SECTOR_SIZE, buffer, sizeof(buffer));
    INode inode;
    inode.inode_construct(inode_number, disk);
    inode.block_number = inode_number;
    inode.permissions = permissions;
    inode.inode_save(disk);
}

void FilesOperation::initialize_rootinode() {
    // this method must be called explicitly right after initializion
    root_inode = inop.inode_allocate(disk);
    printf("Info: root inode number: %llu\n", root_inode);
    init_inode(root_inode, 1);
}

u_int64_t FilesOperation::mkfile(u_int64_t parent_inode_number, const char* name, u_int64_t permissions) {
    // trys to create a file under parent directory
    if (strlen(name)>=56) {
        perror("Name too long, cannot create file or directory");
        return -1;
    }
    INode inode;
    inode.inode_construct(parent_inode_number, disk);
    if (inode.permissions != 1) {
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
    init_inode(new_inode_number, permissions);

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
        if (inode.permissions != 1 || inode.size == 0) {
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

u_int64_t FilesOperation::fischl_mkdir(const char* path, mode_t mode) {
    char *pathdup = strdup(path);
    char *ptr = strrchr(pathdup, '/');
    char *newfilename = ptr+1;
    char *prefix = new char[ptr-pathdup+1];
    for(int i=0;i<ptr-pathdup;i++){
        prefix[i] = pathdup[i];
    }
    prefix[ptr-pathdup] = '\0';
    u_int64_t parent_inode_number = namei(prefix);
    if (parent_inode_number == (u_int64_t)-1) {
        printf("fischl_mkdir failed because namei failed to find inode for %s\n", prefix);
        delete pathdup;
        delete [] prefix;
        return -1;
    }
    u_int64_t ret = mkfile(parent_inode_number, newfilename, 1);
    delete pathdup;
    delete [] prefix;
    return ret;
}

u_int64_t FilesOperation::fischl_mknod(const char* path, mode_t mode) {
    char *pathdup = strdup(path);
    char *ptr = strrchr(pathdup, '/');
    char *newfilename = ptr+1;
    char *prefix = new char[ptr-pathdup+1];
    for(int i=0;i<ptr-pathdup;i++){
        prefix[i] = pathdup[i];
    }
    prefix[ptr-pathdup] = '\0';
    u_int64_t parent_inode_number = namei(prefix);
    if (parent_inode_number == (u_int64_t)-1) {
        printf("fischl_mknod failed because namei failed to find inode for %s", prefix);
        delete pathdup;
        delete [] prefix;
        return -1;
    }
    u_int64_t ret = mkfile(parent_inode_number, newfilename, 0);
    delete pathdup;
    delete [] prefix;
    return ret;
}