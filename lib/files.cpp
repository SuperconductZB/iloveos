//#include "fuse.h" add this when layer3
#include "files.h"
#include <string.h>
#include <sstream>
#include <cassert>

struct DirectoryEntry {
    u_int64_t inode_number;
    char file_name[256];
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
    dotdot.serialize(buffer+264);
    int ret = write_datablock(*inode, 0, buffer);
    inode->inode_save(disk);
}

void FilesOperation::initialize_rootinode() {
    // this method must be called explicitly right after initializion
    u_int64_t root_inode_number = inop.inode_allocate(disk);
    // printf("Info: root inode number: %llu\n", root_inode_number);
    INode *root_inode = new_inode(root_inode_number, S_IFDIR);
    create_dot_dotdot(root_inode, root_inode_number);
    root_node = fischl_init_entry(root_inode_number, "/", root_inode);
    assert(root_node->self_info!=NULL);
    delete root_inode;
}

void FilesOperation::printDirectory(u_int64_t inode_number) {
    // limit to first datablock
    INode inode;
    inode.inode_construct(inode_number, disk);
    char buffer[IO_BLOCK_SIZE] = {0};
    read_datablock(inode, 0, buffer);
    DirectoryEntry ent;
    for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
        ent.deserialize(buffer+i);
        if (ent.inode_number) printf("%s\t%llu;\t", ent.file_name, ent.inode_number);
    }
    printf("\n");
}

INode* FilesOperation::create_new_inode(u_int64_t parent_inode_number, const char* name, mode_t mode) {
    // trys to create a file under parent directory
    if (strlen(name)>=256) {
        perror("Name too long, cannot create file or directory");
        return NULL;
    }
    INode inode;
    inode.inode_construct(parent_inode_number, disk);
    if ((inode.permissions & S_IFMT) != S_IFDIR) {
        fprintf(stderr,"[%s ,%d] please create under directory\n",__func__,__LINE__);
        return NULL;
    }

    // Check if file or directory already exists
    char r_buffer[IO_BLOCK_SIZE] = {0};
    for (u_int64_t idx=0; idx<inode.size; idx++) {
        read_datablock(inode, idx, r_buffer);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(r_buffer+i);
            if (strcmp(ent.file_name, name)==0) {
                if((mode & S_IFMT) == S_IFDIR){
                    fprintf(stderr,"[%s ,%d] %s/ already exists\n",__func__,__LINE__, name);
                }else{
                    fprintf(stderr,"[%s ,%d] %s already exists\n",__func__,__LINE__, name);
                }                  
                return NULL;
            }
        }
    }

    u_int64_t new_inode_number = 0;

    char rw_buffer[IO_BLOCK_SIZE] = {0};
    for (u_int64_t idx=0; idx<inode.size; idx++) {
        read_datablock(inode, idx, rw_buffer);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(rw_buffer+i);
            if (ent.inode_number == 0) {
                new_inode_number = inop.inode_allocate(disk);
                ent.inode_number = new_inode_number;
                strcpy(ent.file_name, name);
                ent.serialize(rw_buffer+i);
                break;
            }
        }
        if (new_inode_number) {
            write_datablock(inode, idx, rw_buffer);
            break;
        }
    }
    
    if (!new_inode_number) {
        char write_buffer[IO_BLOCK_SIZE] = {0};
        DirectoryEntry ent;
        new_inode_number = inop.inode_allocate(disk);
        ent.inode_number = new_inode_number;
        strcpy(ent.file_name, name);
        ent.serialize(write_buffer);
        write_datablock(inode, inode.size, write_buffer);
        inode.inode_save(disk);
    }

    // initialize new file
    INode *get_inode = new_inode(new_inode_number, mode);
    if ((get_inode->permissions & S_IFMT) == S_IFDIR) {
        create_dot_dotdot(get_inode, parent_inode_number);
    }
    return get_inode;
}

u_int64_t FilesOperation::disk_namei(const char* path) {
    // returns the inode number corresponding to path
    u_int64_t current_inode = root_node->self_info->inode_number;
    std::string current_dirname;
    std::istringstream pathStream(path);
	std::string new_name;
    std::getline(pathStream, new_name, '/');
	if(!new_name.empty()){
		printf("disk_namei: path should start with /\n");
		return -1;
	}
	while (std::getline(pathStream, new_name, '/')) {
		INode inode;
        inode.inode_construct(current_inode, disk);
        if ((inode.permissions & S_IFMT) != S_IFDIR || inode.size == 0) {
            printf("disk_namei: %s is not a non-empty directory\n", current_dirname.c_str());
            return -1;
        }
        u_int64_t new_inode_number = 0;

        char buffer[IO_BLOCK_SIZE] = {0};
        for(u_int64_t idx=0; idx<inode.size; idx++) {
            read_datablock(inode, idx, buffer);
            DirectoryEntry ent;
            for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
                ent.deserialize(buffer+i);
                if (strcmp(ent.file_name, new_name.c_str()) == 0) {
                    new_inode_number = ent.inode_number;
                    break;
                }
            }
            if (new_inode_number) break;
        }
        if (!new_inode_number) {
            printf("disk_namei: no name matching %s under directory %s\n", new_name.c_str(), current_dirname.c_str());
            return -1;
        }
        current_inode = new_inode_number;
        current_dirname = new_name;
	}
    return current_inode;
    // path = "/" should return root_inode_number (root_node->self_info->inode_number)
    // path = "/foo.txt" should return inode for foo.txt
    // path = "/mydir" should return inode for mydir
    // path = "/nonemptydir/foo" should return inode for foo
    // path = "/notnonemptydir/foo" should raise error
}

u_int64_t FilesOperation::namei(const char* path) {
    FileNode* filenode = fischl_find_entry(root_node, path);
    if (filenode) return filenode->inode_number;
    else return -1;
}

int FilesOperation::fischl_mkdir(const char* path, mode_t mode) {
    //check path
    char *pathdup = strdup(path);
    char *lastSlash = strrchr(pathdup, '/');
    *lastSlash = '\0'; // Split the string into parent path and new directory name; <parent path>\0<direcotry name>
    char *newDirname = lastSlash+1; //\0<direcotry name>, get from <direcotry name>
    char *ParentPath = pathdup;//pathdup are separated by pathdup, so it take <parent path> only

    FileNode *parent_filenode = strlen(ParentPath)? fischl_find_entry(root_node, ParentPath): root_node->self_info;
    if (parent_filenode == NULL) {
        fprintf(stderr,"[%s ,%d] ParentPath:{%s} not found\n",__func__,__LINE__, ParentPath);
        delete pathdup;
        return -ENOENT;//parentpath directory does not exist
    }
    u_int64_t parent_inode_number = parent_filenode->inode_number;
    //make new inode
    INode* ret = create_new_inode(parent_inode_number, newDirname, mode|S_IFDIR);//specify S_IFDIR as directory
    if (ret == NULL) return -1;//ENOSPC but create_new_inode handle ENAMETOOLONG EEXIST
    fischl_add_entry(parent_filenode->subdirectory, ret->block_number, newDirname, ret);
    delete pathdup;
    return 0;//SUCCESS
}
/*
    special file
*/
int FilesOperation::fischl_mknod(const char* path, mode_t mode, dev_t dev) {
    //check path
    char *pathdup = strdup(path);
    char *lastSlash = strrchr(pathdup, '/');
    *lastSlash = '\0'; // Split the string into parent path and new directory name; <parent path>\0<direcotry name>
    char *newFilename = lastSlash+1; //\0<direcotry name>, get from <direcotry name>
    char *ParentPath = pathdup;//pathdup are separated by pathdup, so it take <parent path> only
    // fprintf(stderr,"[%s ,%d] ParentPath:%s, strlen=%d\n",__func__,__LINE__, ParentPath, strlen(ParentPath));
    FileNode *parent_filenode = strlen(ParentPath)? fischl_find_entry(root_node, ParentPath): root_node->self_info;
    if (parent_filenode == NULL) {
        fprintf(stderr,"[%s ,%d] ParentPath:{%s} not found\n",__func__,__LINE__, ParentPath);
        delete pathdup;
        return -1;
    }
    u_int64_t parent_inode_number = parent_filenode->inode_number;
    //make new inode
    INode* ret = create_new_inode(parent_inode_number, newFilename, mode);
    if (ret == NULL) return -1;//ENOSPC but create_new_inode handle ENAMETOOLONG EEXIST
    //make new node
    fischl_add_entry(parent_filenode->subdirectory, ret->block_number, newFilename, ret);
    delete pathdup;
    return 0;//SUCESS
}
/*
    regular file
*/
int FilesOperation::fischl_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    //check path
    char *pathdup = strdup(path);
    char *lastSlash = strrchr(pathdup, '/');
    *lastSlash = '\0'; // Split the string into parent path and new directory name; <parent path>\0<direcotry name>
    char *newFilename = lastSlash+1; //\0<direcotry name>, get from <direcotry name>
    char *ParentPath = pathdup;//pathdup are separated by pathdup, so it take <parent path> only
    // fprintf(stderr,"[%s ,%d] ParentPath:%s, strlen=%d\n",__func__,__LINE__, ParentPath, strlen(ParentPath));
    FileNode *parent_filenode = strlen(ParentPath)? fischl_find_entry(root_node, ParentPath): root_node->self_info;
    if (parent_filenode == NULL) {
        fprintf(stderr,"[%s ,%d] ParentPath:{%s} not found\n",__func__,__LINE__, ParentPath);
        delete pathdup;
        return -1;
    }
    u_int64_t parent_inode_number = parent_filenode->inode_number;
    //make new inode
    INode* ret = create_new_inode(parent_inode_number, newFilename, mode);
    if (ret == NULL) return -1;//ENOSPC but create_new_inode handle ENAMETOOLONG EEXIST
    //make new node in RAM
    fischl_add_entry(parent_filenode->subdirectory, ret->block_number, newFilename, ret);
    //directly give inode number rather than create file descriptor table
    fi->fh = ret->block_number;//assign file descriptor as inode number to fuse system
    delete pathdup;
    return 0;//SUCESS
}

void FilesOperation::unlink_inode(u_int64_t inode_number) {
    INode inode;
    inode.inode_construct(inode_number, disk);
    if ((inode.permissions & S_IFMT) == S_IFDIR) {
        char buffer[IO_BLOCK_SIZE] = {0};
        for(u_int64_t idx=0; idx<inode.size; idx++) {
            read_datablock(inode, idx, buffer);
            DirectoryEntry ent;
            for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
                if(ent.inode_number && strcmp(ent.file_name,".") && strcmp(ent.file_name,"..")){
                    unlink_inode(ent.inode_number);
                }
            }
        }
    }
    while(inode.size != 0) {
        inode.datablock_deallocate(disk);
        inode.size--;
    }
    inop.inode_free(disk, inode_number);
}

int FilesOperation::fischl_unlink(const char* path) {
    char *pathdup = strdup(path);
    char *lastSlash = strrchr(pathdup, '/');
    *lastSlash = '\0';
    char *filename = lastSlash+1;
    char *ParentPath = pathdup;
    if (!strcmp(filename,".")||!strcmp(filename,"..")) {
        printf("refusing to remove . or ..\n");
        return -1;
    }
    FileNode *parent_filenode = fischl_find_entry(root_node, ParentPath);
    if (parent_filenode == NULL) {
        printf("parent %s not found by fischl_find_entry\n", ParentPath);
        delete pathdup;
        return -1;
    }
    u_int64_t parent_inode_number = parent_filenode->inode_number;
    u_int64_t target_inode = 0;
    
    // remove its record from parent
    INode parent_INode;
    parent_INode.inode_construct(parent_inode_number, disk);
    char rw_buffer[IO_BLOCK_SIZE] = {0};
    for (u_int64_t idx=0; idx<parent_INode.size; idx++) {
        read_datablock(parent_INode, idx, rw_buffer);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(rw_buffer+i);
            if (strcmp(ent.file_name, filename)==0) {
                target_inode = ent.inode_number;
                ent.inode_number = 0;
                ent.serialize(rw_buffer+i);
                break;
            }
        }
        if (target_inode) {
            write_datablock(parent_INode, idx, rw_buffer);
            break;
        }
    }
    
    // remove inode itself
    if (target_inode) {
        unlink_inode(target_inode);
        // remove node itself and from parent hash
        fischl_rm_entry(parent_filenode->subdirectory, filename);
        delete pathdup;
        return 0;
    } else {
        printf("cannot find %s in %s", filename, ParentPath);
        delete pathdup;
        return -1;
    }
}

int FilesOperation::fischl_open(const char *path, struct fuse_file_info *fi){
    /*Creation (O_CREAT, O_EXCL, O_NOCTTY) flags will be filtered out / handled by the kernel. 
     if no files will use create function
    */
    FileNode *get_file;
    if((get_file = fischl_find_entry(root_node, path)) == NULL)
        return -ENOENT;
    //if need to do with flag fi->flags ((fi->flags & O_ACCMODE)). Initial setting ALL access
    //create function will handle file descriptor fi->fh
    fi->fh = get_file->inode_number;
    return 0;//SUCESS
}

int FilesOperation::fischl_release(const char *path, struct fuse_file_info *fi){
    /*Creation (O_CREAT, O_EXCL, O_NOCTTY) flags will be filtered out / handled by the kernel. 
     if no files will use create function
    */
    FileNode *get_file;
    if((get_file = fischl_find_entry(root_node, path)) == NULL)
        return -ENOENT;
    //do with file descriptor that cannot be used
    fi->fh = -1;
    return 0;//SUCESS
}

int FilesOperation::fischl_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    /** Write data to an open file
	 *
	 * Write should return exactly the number of bytes requested
	 * except on error.	 An exception to this is when the 'direct_io'
	 * mount option is specified (see read operation).
	 *
	 * Unless FUSE_CAP_HANDLE_KILLPRIV is disabled, this method is
	 * expected to reset the setuid and setgid bits.
	 */
    // use path for debug, filedescriptor is enough
    // FileNode *get_file;
    // if((get_file = fischl_find_entry(root_node, path)) == NULL)
    //     return -ENOENT;
    // Caution! this based on content in file are multiple of IO_BLOCK_SIZE, not the exact write size.
    // based on current write_datablock API implement, when write_datablock pass with actual size not index this function should be fixed
    INode inode;
    // Assuming inode is correctly initialized here based on 'path'
    inode.inode_construct(fi->fh, disk);
    size_t len = inode.size * IO_BLOCK_SIZE;  // Assuming each block is 4096 bytes

    size_t bytes_write = 0;
    size_t block_index = offset / IO_BLOCK_SIZE;  // Starting block index
    size_t block_offset = offset % IO_BLOCK_SIZE; // Offset within the first block
    while (bytes_write < size) {
        char block_buffer[IO_BLOCK_SIZE];  // Temporary buffer for each block
        size_t copy_size = std::min(size - bytes_write, IO_BLOCK_SIZE - block_offset);
        memcpy(block_buffer + block_offset, buf + bytes_write, copy_size);
        write_datablock(inode, block_index, block_buffer);
        fprintf(stderr,"[%s ,%d] inode.size %d\n",__func__,__LINE__, inode.size);
        fprintf(stderr,"[%s ,%d] block_index %d\n",__func__,__LINE__, block_index);
        fprintf(stderr,"[%s ,%d] buf %s, block_buffer %s\n",__func__,__LINE__, buf, block_buffer);
        bytes_write += copy_size;
        block_index++;
        block_offset = 0;  // Only the first block might have a non-zero offset
    }

    return bytes_write;  // Return the actual number of bytes read
}

int FilesOperation::fischl_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    /** Read data from an open file
	 *
	 * Read should return exactly the number of bytes requested except
	 * on EOF or error, otherwise the rest of the data will be
	 * substituted with zeroes.	 An exception to this is when the
	 * 'direct_io' mount option is specified, in which case the return
	 * value of the read system call will reflect the return value of
	 * this operation.
	 */
    // Caution! this based on content in file are multiple of IO_BLOCK_SIZE, not the exact write size.
    // based on current read_datablock API implement, when read_datablock pass with actual size not index this function should be fixed 
    INode inode;
    // Assuming inode is correctly initialized here based on 'path'
    inode.inode_construct(fi->fh, disk);
    size_t len = inode.size * IO_BLOCK_SIZE;  // Assuming each block is 4096 bytes

    if (offset >= len) return 0;  // Offset is beyond the end of the file
    if (offset + size > len) size = len - offset;  // Adjust size if it goes beyond EOF

    size_t bytes_read = 0;
    size_t block_index = offset / IO_BLOCK_SIZE;  // Starting block index
    size_t block_offset = offset % IO_BLOCK_SIZE; // Offset within the first block
    // fprintf(stderr,"[%s ,%d] inode.size %d\n",__func__,__LINE__, inode.size);
    while (bytes_read < size && block_index < inode.size) {
        char block_buffer[IO_BLOCK_SIZE];  // Temporary buffer for each block
        read_datablock(inode, block_index, block_buffer);
        // fprintf(stderr,"[%s ,%d] block_index %d\n",__func__,__LINE__, block_index);
        size_t copy_size = std::min(size - bytes_read, IO_BLOCK_SIZE - block_offset);
        memcpy(buf + bytes_read, block_buffer + block_offset, copy_size);
        // fprintf(stderr,"[%s ,%d] buf %s, block_buffer %s\n",__func__,__LINE__, buf, block_buffer);
        bytes_read += copy_size;
        block_index++;
        block_offset = 0;  // Only the first block might have a non-zero offset
    }

    return bytes_read;  // Return the actual number of bytes read
}