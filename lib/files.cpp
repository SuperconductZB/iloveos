//#include "fuse.h" add this when layer3
#define FUSE_USE_VERSION 31

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

void FilesOperation::printbuffer(const char* buff, int len) {
    for(int i=0;i<len;i++){
        printf("%x ",buff[i]);
    }
    printf("\n");
}

FilesOperation::FilesOperation(RawDisk& disk_, Fs* fs): disk(disk_) {
    this->fs = fs;
}

void FilesOperation::create_dot_dotdot(INode_Data* inode, u_int64_t parent_inode_number) {
    char buffer[IO_BLOCK_SIZE] = {0};
    DirectoryEntry dot;
    dot.inode_number = inode->inode_num;
    strcpy(dot.file_name, ".");
    dot.serialize(buffer);
    DirectoryEntry dotdot;
    dotdot.inode_number = parent_inode_number;
    strcpy(dotdot.file_name, "..");
    dotdot.serialize(buffer+264);
    int ret = fs->write(inode, buffer, IO_BLOCK_SIZE, 0);
    //printf("in create_dot_dotdot: fs->write returned %d\n",ret);
}

void FilesOperation::initialize_rootinode() {
    // this method must be called explicitly right after initializion
    INode_Data *root_inode = new INode_Data();
    fs->inode_manager->new_inode(0, 0, S_IFDIR, root_inode);
    u_int64_t root_inode_number = root_inode->inode_num;
    create_dot_dotdot(root_inode, root_inode_number);
    root_node = fischl_init_entry(root_inode_number, "/", root_inode);
    assert(root_node->self_info!=NULL);
    fs->inode_manager->save_inode(root_inode);
}

void FilesOperation::printDirectory(u_int64_t inode_number) {
    INode_Data inode;
    inode.inode_num = inode_number;
    fs->inode_manager->load_inode(&inode);
    char buffer[IO_BLOCK_SIZE] = {0};
    for (u_int64_t idx=0; idx<inode.metadata.size/IO_BLOCK_SIZE; idx++) {
        fs->read(&inode, buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(buffer+i);
            if (ent.inode_number) printf("%s\t%llu;\t", ent.file_name, ent.inode_number);
        }
    }
    printf("\n");
}

INode_Data* FilesOperation::create_new_inode(u_int64_t parent_inode_number, const char* name, mode_t mode) {
    // trys to create a file under parent directory
    if (strlen(name)>=256) {
        perror("Name too long, cannot create file or directory");
        return NULL;
    }
    INode_Data inode;
    inode.inode_num = parent_inode_number;
    fs->inode_manager->load_inode(&inode);
    if ((inode.metadata.permissions & S_IFMT) != S_IFDIR) {
        fprintf(stderr,"[%s ,%d] please create under directory\n",__func__,__LINE__);
        return NULL;
    }

    // Check if file or directory already exists
    char r_buffer[IO_BLOCK_SIZE] = {0};
    for (u_int64_t idx=0; idx<inode.metadata.size/IO_BLOCK_SIZE; idx++) {
        fs->read(&inode, r_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(r_buffer+i);
            if (strcmp(ent.file_name, name)==0 && ent.inode_number != 0) {
                if((mode & S_IFMT) == S_IFDIR){
                    fprintf(stderr,"[%s ,%d] %s/ already exists\n",__func__,__LINE__, name);
                }else{
                    fprintf(stderr,"[%s ,%d] %s already exists\n",__func__,__LINE__, name);
                }                  
                return NULL;
            }
        }
    }

    bool allocated = false;
    INode_Data *new_inode = new INode_Data();
    fs->inode_manager->new_inode(0, 0, mode, new_inode);
    if ((mode & S_IFMT) == S_IFDIR) {
        create_dot_dotdot(new_inode, parent_inode_number);
        fs->inode_manager->save_inode(new_inode);
    }

    char rw_buffer[IO_BLOCK_SIZE] = {0};
    for (u_int64_t idx=0; idx<inode.metadata.size/IO_BLOCK_SIZE; idx++) {
        fs->read(&inode, rw_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(rw_buffer+i);
            if (ent.inode_number == 0) {
                allocated = true;
                ent.inode_number = new_inode->inode_num;
                strcpy(ent.file_name, name);
                ent.serialize(rw_buffer+i);
                break;
            }
        }
        if (allocated) {
            fs->write(&inode, rw_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
            break;
        }
    }

    if (!allocated) {
        char write_buffer[IO_BLOCK_SIZE] = {0};
        DirectoryEntry ent;
        ent.inode_number = new_inode->inode_num;
        strcpy(ent.file_name, name);
        ent.serialize(write_buffer);
        fs->write(&inode, write_buffer, IO_BLOCK_SIZE, (inode.metadata.size/IO_BLOCK_SIZE)*IO_BLOCK_SIZE);
        fs->inode_manager->save_inode(&inode);
    }

    return new_inode;
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
		INode_Data inode;
        inode.inode_num = current_inode;
        fs->inode_manager->load_inode(&inode);
        if ((inode.metadata.permissions & S_IFMT) != S_IFDIR || inode.metadata.size == 0) {
            printf("disk_namei: %s is not a non-empty directory\n", current_dirname.c_str());
            return -1;
        }
        u_int64_t new_inode_number = 0;

        char buffer[IO_BLOCK_SIZE] = {0};
        for(u_int64_t idx=0; idx<inode.metadata.size/IO_BLOCK_SIZE; idx++) {
            fs->read(&inode, buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
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
        free(pathdup);
        return -ENOENT;//parentpath directory does not exist
    }
    u_int64_t parent_inode_number = parent_filenode->inode_number;
    //printf("%s, %llu, %s\n", parent_filenode->name, parent_inode_number, newDirname);
    //make new inode
    INode_Data* ret = create_new_inode(parent_inode_number, newDirname, mode|S_IFDIR);//specify S_IFDIR as directory
    if (ret == NULL) return -1;//ENOSPC but create_new_inode handle ENAMETOOLONG EEXIST
    fischl_add_entry(parent_filenode->subdirectory, ret->inode_num, newDirname, ret);
    free(pathdup);
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
        free(pathdup);
        return -1;
    }
    u_int64_t parent_inode_number = parent_filenode->inode_number;
    //make new inode
    INode_Data* ret = create_new_inode(parent_inode_number, newFilename, mode);
    if (ret == NULL) return -1;//ENOSPC but create_new_inode handle ENAMETOOLONG EEXIST
    //make new node
    fischl_add_entry(parent_filenode->subdirectory, ret->inode_num, newFilename, ret);
    free(pathdup);
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
        free(pathdup);
        return -1;
    }
    u_int64_t parent_inode_number = parent_filenode->inode_number;
    //make new inode
    INode_Data* ret = create_new_inode(parent_inode_number, newFilename, mode);
    if (ret == NULL) return -1;//ENOSPC but create_new_inode handle ENAMETOOLONG EEXIST
    //make new node in RAM
    fischl_add_entry(parent_filenode->subdirectory, ret->inode_num, newFilename, ret);
    //directly give inode number rather than create file descriptor table
    fi->fh = ret->inode_num;//assign file descriptor as inode number to fuse system
    free(pathdup);
    return 0;//SUCESS
}

int FilesOperation::fischl_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {

    (void) fi;
	int res = 0;
    u_int64_t fh = namei(path);

    if (fh == -1){
        return -ENOENT;
    }

    INode_Data inode;
    inode.inode_num = fh;
    fs->inode_manager->load_inode(&inode);

	//memset(stbuf, 0, sizeof(struct stat));
	if ((inode.metadata.permissions & S_IFMT) == S_IFDIR) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;//inode.metadata.reference_count;
        stbuf->st_uid = inode.metadata.uid;
        stbuf->st_gid = inode.metadata.gid;
        stbuf->st_ino = inode.inode_num;
	} else if(S_ISLNK(inode.metadata.permissions)){
        printf("THIS IS GOOD %d %llu\n", inode.metadata.size, inode.inode_num);
        stbuf->st_mode = S_IFLNK;
		stbuf->st_nlink = 1;//inode.metadata.reference_count;
        stbuf->st_uid = inode.metadata.uid;
        stbuf->st_gid = inode.metadata.gid;
        stbuf->st_size = inode.metadata.size;
        stbuf->st_ino = inode.inode_num;
    } else {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = inode.metadata.reference_count;
        stbuf->st_uid = inode.metadata.uid;
        stbuf->st_gid = inode.metadata.gid;
		stbuf->st_size = inode.metadata.size;
        stbuf->st_ino = inode.inode_num;
	}
    perror(std::to_string(inode.inode_num).c_str());
	return res;
}

int FilesOperation::fischl_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t ft, struct fuse_file_info *fi, enum fuse_readdir_flags flg) {
	    //check path
    u_int64_t fh = namei(path);

    if (fh == -1){
        return -1;
    }

    INode_Data inode;
    inode.inode_num = fh;
    fs->inode_manager->load_inode(&inode);
    char buffer[IO_BLOCK_SIZE] = {0};
    for (u_int64_t idx=0; idx<inode.metadata.size/IO_BLOCK_SIZE; idx++) {
        fs->read(&inode, buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(buffer+i);
            if (ent.inode_number) {
                filler(buf, ent.file_name, NULL, 0, FUSE_FILL_DIR_PLUS);
                //printf("%s\t%llu;\t", ent.file_name, ent.inode_number);
            }
        }
    }

    return 0;
}

int FilesOperation::fischl_releasedir(const char *path, struct fuse_file_info *fi){
    if(fischl_find_entry(root_node, path) == NULL)
        return -ENOENT;
    //do with file descriptor that cannot be used
    fi->fh = -1;
    return 0;//SUCESS
}

void FilesOperation::unlink_inode(u_int64_t inode_number) {
    INode_Data inode;
    inode.inode_num = inode_number;
    fs->inode_manager->load_inode(&inode);
    if (inode.metadata.reference_count > 1 && (inode.metadata.permissions & S_IFMT) != S_IFDIR){
        inode.metadata.reference_count -= 1;
        fs->inode_manager->save_inode(&inode);
        return;
    }
    if ((inode.metadata.permissions & S_IFMT) == S_IFDIR) {
        char buffer[IO_BLOCK_SIZE] = {0};
        for(u_int64_t idx=0; idx<inode.metadata.size/IO_BLOCK_SIZE; idx++) {
            fs->read(&inode, buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
            DirectoryEntry ent;
            for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
                if(ent.inode_number && strcmp(ent.file_name,".") && strcmp(ent.file_name,"..")){
                    unlink_inode(ent.inode_number);
                }
            }
        }
    }
    // TODO: This is probably incorrect
    // size is unsigned int
    while(inode.metadata.size != 0) {
        printf("dealloc, %d\n", inode.metadata.size);
        u_int64_t dummy;
        fs->deallocate_datablock(&inode, &dummy);
        if (inode.metadata.size < IO_BLOCK_SIZE){
            inode.metadata.size = 0;
            break;
        }
        inode.metadata.size-=IO_BLOCK_SIZE;
    }
    fs->inode_manager->free_inode(&inode);
}

int FilesOperation::fischl_rmdir(const char* path) {
    char *pathdup = strdup(path);
    char *lastSlash = strrchr(pathdup, '/');
    *lastSlash = '\0';
    char *dirname = lastSlash+1;
    char *ParentPath = pathdup;
    if (!strcmp(dirname,".")||!strcmp(dirname,"..")) {
        printf("refusing to remove . or ..\n");
        return -1;
    }
    FileNode *parent_filenode = fischl_find_entry(root_node, ParentPath);
    if (parent_filenode == NULL) {
        printf("parent %s not found by fischl_find_entry\n", ParentPath);
        free(pathdup);
        return -1;
    }
    u_int64_t parent_inode_number = parent_filenode->inode_number;
    u_int64_t target_inode = 0;
    
    // remove its record from parent
    INode_Data parent_INode;
    parent_INode.inode_num = parent_inode_number;
    fs->inode_manager->load_inode(&parent_INode);
    char rw_buffer[IO_BLOCK_SIZE] = {0};
    for (u_int64_t idx=0; idx<parent_INode.metadata.size/IO_BLOCK_SIZE; idx++) {
        fs->read(&parent_INode, rw_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(rw_buffer+i);
            if (strcmp(ent.file_name, dirname)==0) {
                target_inode = ent.inode_number;
                ent.inode_number = 0;
                memset(ent.file_name, 0, sizeof(ent.file_name));
                ent.serialize(rw_buffer+i);
                break;
            }
        }
        if (target_inode) {
            fs->write(&parent_INode, rw_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
            break;
        }
    }
    
    // remove inode itself
    if (target_inode) {
        unlink_inode(target_inode);
        // remove node itself and from parent hash
        fischl_rm_entry(parent_filenode->subdirectory, dirname);
        free(pathdup);
        return 0;
    } else {
        printf("cannot find %s in %s", dirname, ParentPath);
        free(pathdup);
        return -1;
    }
}

int FilesOperation::fischl_chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
    (void) fi;
	int res = 0;
    u_int64_t fh = namei(path);

    if (fh == -1){
        return -ENOENT;
    }

    INode_Data inode;
    inode.inode_num = fh;
    fs->inode_manager->load_inode(&inode);
    inode.metadata.permissions = mode;
    fs->inode_manager->save_inode(&inode);
    return 0;
}

int FilesOperation::fischl_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    (void) fi;
	int res = 0;
    u_int64_t fh = namei(path);

    if (fh == -1){
        return -ENOENT;
    }

    INode_Data inode;
    inode.inode_num = fh;
    fs->inode_manager->load_inode(&inode);
    inode.metadata.uid = uid;
    inode.metadata.gid = gid;
    fs->inode_manager->save_inode(&inode);
    return 0;
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
        free(pathdup);
        return -1;
    }
    u_int64_t parent_inode_number = parent_filenode->inode_number;
    u_int64_t target_inode = 0;
    
    // remove its record from parent
    INode_Data parent_INode;
    parent_INode.inode_num = parent_inode_number;
    fs->inode_manager->load_inode(&parent_INode);
    char rw_buffer[IO_BLOCK_SIZE] = {0};
    for (u_int64_t idx=0; idx<parent_INode.metadata.size/IO_BLOCK_SIZE; idx++) {
        fs->read(&parent_INode, rw_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(rw_buffer+i);
            if (strcmp(ent.file_name, filename)==0) {
                target_inode = ent.inode_number;
                ent.inode_number = 0;
                memset(ent.file_name, 0, sizeof(ent.file_name));
                ent.serialize(rw_buffer+i);
                break;
            }
        }
        if (target_inode) {
            fs->write(&parent_INode, rw_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
            break;
        }
    }
    
    // remove inode itself
    if (target_inode) {
        unlink_inode(target_inode);
        // remove node itself and from parent hash
        fischl_rm_entry(parent_filenode->subdirectory, filename);
        free(pathdup);
        return 0;
    } else {
        printf("cannot find %s in %s", filename, ParentPath);
        free(pathdup);
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
    
    INode_Data inode;
    // Assuming inode is correctly initialized here based on 'path'
    inode.inode_num = fi->fh;
    fs->inode_manager->load_inode(&inode);
    //size_t len = (inode.metadata.size/IO_BLOCK_SIZE) * IO_BLOCK_SIZE;  // Assuming each block is 4096 bytes
    // Determine the length of the buffer
    // Allocate memory for the new buffer
    char* buffer = (char*)malloc(size);
    memcpy(buffer, buf, size);
    size_t bytes_write = fs->write(&inode, buffer, size, offset);
    /*size_t block_index = offset / IO_BLOCK_SIZE;  // Starting block index
    size_t block_offset = offset % IO_BLOCK_SIZE; // Offset within the first block
    while (bytes_write < size) {
        char block_buffer[IO_BLOCK_SIZE];  // Temporary buffer for each block
        size_t copy_size = std::min(size - bytes_write, IO_BLOCK_SIZE - block_offset);
        memcpy(block_buffer + block_offset, buf + bytes_write, copy_size);
        fs->write(&inode, block_buffer, IO_BLOCK_SIZE, block_index*IO_BLOCK_SIZE);
        // fprintf(stderr,"[%s ,%d] inode.size %d, block_index %d, block_buffer %s\n",__func__,__LINE__, inode.size, block_index, block_buffer);
        bytes_write += copy_size;
        block_index++;
        block_offset = 0;  // Only the first block might have a non-zero offset
    }*/
    fs->inode_manager->save_inode(&inode);
    free(buffer);
    return bytes_write;  // Return the actual number of bytes read
}

int FilesOperation::fischl_readlink(const char* path, char* buf, size_t size){
    FileNode *get_file;
    if((get_file = fischl_find_entry(root_node, path)) == NULL)
        return -ENOENT;
    INode_Data symlink_inode;
    symlink_inode.inode_num = get_file->inode_number;
    fs->inode_manager->load_inode(&symlink_inode);
    //char buffer[symlink_inode.metadata.size];
    //memset(buffer, 0, sizeof(buffer));
    fs->read(&symlink_inode, buf, symlink_inode.metadata.size, 0);
    printf("READLINK %d %s\n", symlink_inode.metadata.size, buf);
    /*u_int64_t fh = namei(buffer);
    if (fh == -1){
        return -ENOENT;
    }
    INode_Data inode;
    // Assuming inode is correctly initialized here based on 'path'
    inode.inode_num = fh;
    fs->inode_manager->load_inode(&inode);
    size_t bytes_read = fs->read(&inode, buf, size, 0);
    printf("READLINK %d %s\n", bytes_read, buf);*/
    return 0;
}

int FilesOperation::fischl_symlink(const char* to, const char* from){
    //check path
    //printf("SYMLINK %s %s\n", from, to);
    char *pathdup = strdup(from);
    char *lastSlash = strrchr(pathdup, '/');
    *lastSlash = '\0'; // Split the string into parent path and new directory name; <parent path>\0<direcotry name>
    char *newFilename = lastSlash+1; //\0<direcotry name>, get from <direcotry name>
    char *ParentPath = pathdup;//pathdup are separated by pathdup, so it take <parent path> only
    // fprintf(stderr,"[%s ,%d] ParentPath:%s, strlen=%d\n",__func__,__LINE__, ParentPath, strlen(ParentPath));
    FileNode *parent_filenode = strlen(ParentPath)? fischl_find_entry(root_node, ParentPath): root_node->self_info;
    if (parent_filenode == NULL) {
        fprintf(stderr,"[%s ,%d] ParentPath:{%s} not found\n",__func__,__LINE__, ParentPath);
        free(pathdup);
        return -1;
    }
    u_int64_t parent_inode_number = parent_filenode->inode_number;
    //make new inode
    INode_Data* ret = create_new_inode(parent_inode_number, newFilename, S_IFLNK);
    if (ret == NULL) return -1;//ENOSPC but create_new_inode handle ENAMETOOLONG EEXIST
    //make new node in RAM
    fischl_add_entry(parent_filenode->subdirectory, ret->inode_num, newFilename, ret);
    size_t size = strlen(to);
    char* buffer = (char*)malloc(size);
    memcpy(buffer, to, size);
    //printf("%d %s\n", size, buffer);
    size_t bytes_write = fs->write(ret, buffer, size, 0);
    free(buffer);
    free(pathdup);
    fs->inode_manager->save_inode(ret);
    //printf("%d %d %llu\n", bytes_write, ret->metadata.size, ret->inode_num);
    return 0;//SUCESS
}

int FilesOperation::insert_inode_to(u_int64_t parent_inode_number, const char* name, INode_Data *new_inode) {
    // trys to create a file under parent directory
    if (strlen(name)>=256) {
        perror("Name too long, cannot create file or directory");
        return -1;
    }
    INode_Data inode;
    inode.inode_num = parent_inode_number;
    fs->inode_manager->load_inode(&inode);
    if ((inode.metadata.permissions & S_IFMT) != S_IFDIR) {
        fprintf(stderr,"[%s ,%d] please create under directory\n",__func__,__LINE__);
        return -1;
    }

    // Check if file or directory already exists
    char r_buffer[IO_BLOCK_SIZE] = {0};
    for (u_int64_t idx=0; idx<inode.metadata.size/IO_BLOCK_SIZE; idx++) {
        fs->read(&inode, r_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(r_buffer+i);
            if (strcmp(ent.file_name, name)==0 && ent.inode_number != 0) {
                if((new_inode->metadata.permissions & S_IFMT) == S_IFDIR){
                    fprintf(stderr,"[%s ,%d] %s/ already exists\n",__func__,__LINE__, name);
                }else{
                    fprintf(stderr,"[%s ,%d] %s already exists\n",__func__,__LINE__, name);
                }                  
                return -1;
            }
        }
    }

    bool allocated = false;

    char rw_buffer[IO_BLOCK_SIZE] = {0};
    for (u_int64_t idx=0; idx<inode.metadata.size/IO_BLOCK_SIZE; idx++) {
        fs->read(&inode, rw_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(rw_buffer+i);
            if (ent.inode_number == 0) {
                allocated = true;
                ent.inode_number = new_inode->inode_num;
                strcpy(ent.file_name, name);
                ent.serialize(rw_buffer+i);
                break;
            }
        }
        if (allocated) {
            fs->write(&inode, rw_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
            break;
        }
    }

    if (!allocated) {
        char write_buffer[IO_BLOCK_SIZE] = {0};
        DirectoryEntry ent;
        ent.inode_number = new_inode->inode_num;
        strcpy(ent.file_name, name);
        ent.serialize(write_buffer);
        fs->write(&inode, write_buffer, IO_BLOCK_SIZE, (inode.metadata.size/IO_BLOCK_SIZE)*IO_BLOCK_SIZE);
        fs->inode_manager->save_inode(&inode);
    }

    return 0;
}

int FilesOperation::fischl_link(const char* from, const char* to){

    FileNode *get_file;
    if((get_file = fischl_find_entry(root_node, from)) == NULL)
        return -ENOENT;
    INode_Data ret;
    ret.inode_num = get_file->inode_number;
    fs->inode_manager->load_inode(&ret);
    
    //check path
    char *pathdup = strdup(to);
    char *lastSlash = strrchr(pathdup, '/');
    *lastSlash = '\0'; // Split the string into parent path and new directory name; <parent path>\0<direcotry name>
    char *newFilename = lastSlash+1; //\0<direcotry name>, get from <direcotry name>
    char *ParentPath = pathdup;//pathdup are separated by pathdup, so it take <parent path> only
    // fprintf(stderr,"[%s ,%d] ParentPath:%s, strlen=%d\n",__func__,__LINE__, ParentPath, strlen(ParentPath));
    FileNode *parent_filenode = strlen(ParentPath)? fischl_find_entry(root_node, ParentPath): root_node->self_info;
    if (parent_filenode == NULL) {
        fprintf(stderr,"[%s ,%d] ParentPath:{%s} not found\n",__func__,__LINE__, ParentPath);
        free(pathdup);
        return -1;
    }

    u_int64_t parent_inode_number = parent_filenode->inode_number;
    if(insert_inode_to(parent_inode_number, newFilename, &ret)<0){
            return -1;
        }

    ret.metadata.reference_count += 1;
    fs->inode_manager->save_inode(&ret);
    fischl_add_entry(parent_filenode->subdirectory, ret.inode_num, newFilename, &ret);
    free(pathdup);
    return 0;
}


// TODO: rename dir and rename fail
int FilesOperation::fischl_rename(const char *old_path, const char *new_path, unsigned int flags){
    //find old path
    char *pathdup = strdup(old_path);
    char *lastSlash = strrchr(pathdup, '/');
    *lastSlash = '\0';
    char *filename = lastSlash+1;
    char *oldParentPath = pathdup;
    if (!strcmp(filename,".")||!strcmp(filename,"..")) {
        printf("refuse to remove . or ..\n");
        return -1;
    }
    //put old path info in rename struct
    RenameInfo rename_info;
    rename_info.exchangeExist = false;
    rename_info.oldParentNode = fischl_find_entry(root_node, oldParentPath);
    rename_info.oldFileNode   = fischl_find_entry(rename_info.oldParentNode->subdirectory, filename);

    if (rename_info.oldFileNode == NULL) {
        printf("path %s not found by fischl_find_entry\n", old_path);
        free(pathdup);
        return -1;
    }
    
    //find new path
    char *new_pathdup = strdup(new_path);
    lastSlash = strrchr(new_pathdup, '/');
    *lastSlash = '\0'; // Split the string into parent path and new directory name; <parent path>\0<direcotry name>
    char *newParentPath = new_pathdup;//pathdup are separated by pathdup, so it take <parent path> only
    //put new path info in rename struct
    rename_info.newName       = lastSlash+1; //\0<direcotry name>, get from <direcotry name>
    rename_info.newFileNode   = fischl_find_entry(root_node, new_path);
    rename_info.newParentNode = strlen(newParentPath)? fischl_find_entry(root_node, newParentPath): root_node->self_info;

    //configure with flag
    if (flags & RENAME_NOREPLACE) {
        // Check if newpath exists and return error if it does
        if(rename_info.newFileNode != NULL){
            fprintf(stderr,"[%s ,%d] newpath has already existed\n",__func__,__LINE__);
            free(new_pathdup);//new path
            free(pathdup);//old path
            return -1;
        }
    }

    if (flags & RENAME_EXCHANGE) {
        // Ensure both oldpath and newpath exist and exchange them atomically
        if(rename_info.newFileNode == NULL){
            fprintf(stderr,"[%s ,%d] newpath does not exist cannot exchange\n",__func__,__LINE__);
            free(new_pathdup);
            free(pathdup);
            return -1;
        }
        rename_info.exchangeExist = true;
    }

    // Normal rename logic if no flags are specified; can overwirte
    // Hard Disk rename

    // remove its record from parent
    INode_Data parent_INode;
    parent_INode.inode_num = rename_info.oldParentNode->inode_number;
    fs->inode_manager->load_inode(&parent_INode);
    char rw_buffer[IO_BLOCK_SIZE] = {0};
    
    // relocate the old path file to new path
    INode_Data ret;
    ret.inode_num = rename_info.oldFileNode->inode_number;
    fs->inode_manager->load_inode(&ret);
    if(insert_inode_to(rename_info.newParentNode->inode_number, rename_info.newName, &ret)<0){
        return -1;
    }
    bool change_flag = false;
    //delete record on old path
    for (u_int64_t idx=0; idx<parent_INode.metadata.size/IO_BLOCK_SIZE; idx++) {
        fs->read(&parent_INode, rw_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
        DirectoryEntry ent;
        for(int i=0;i<=IO_BLOCK_SIZE-264;i+=264){
            ent.deserialize(rw_buffer+i);
            if (strcmp(ent.file_name, filename)==0) {
                change_flag = true;//should be change
                ent.inode_number = 0;
                memset(ent.file_name, 0, sizeof(ent.file_name));
                ent.serialize(rw_buffer+i);
                break;
            }
        }
        if (change_flag) {
            fs->write(&parent_INode, rw_buffer, IO_BLOCK_SIZE, idx*IO_BLOCK_SIZE);
            change_flag = false;
            break;
        }
    }
    
    //free path
    free(pathdup);
    free(new_pathdup);
    return 0;
}

int FilesOperation::fischl_truncate(const char *path, off_t offset, struct fuse_file_info *fi){
    (void) fi;
	int res = 0;
    u_int64_t fh = namei(path);

    if (fh == -1){
        return -ENOENT;
    }

    INode_Data inode;
    inode.inode_num = fh;
    fs->inode_manager->load_inode(&inode);
    while(inode.metadata.size > offset + IO_BLOCK_SIZE) {
        printf("dealloc, %d\n", inode.metadata.size);
        u_int64_t dummy;
        fs->deallocate_datablock(&inode, &dummy);
        if (inode.metadata.size < IO_BLOCK_SIZE){
            inode.metadata.size = 0;
            break;
        }
        inode.metadata.size-=IO_BLOCK_SIZE;
    }
    inode.metadata.size = offset;
    fs->inode_manager->save_inode(&inode);
    return 0;
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
    INode_Data inode;
    // Assuming inode is correctly initialized here based on 'path'
    inode.inode_num = fi->fh;
    fs->inode_manager->load_inode(&inode);
    size_t bytes_read = fs->read(&inode, buf, size, offset);
    /*size_t len = (inode.metadata.size/IO_BLOCK_SIZE) * IO_BLOCK_SIZE;  // Assuming each block is 4096 bytes

    if (offset >= len) return 0;  // Offset is beyond the end of the file
    if (offset + size > len) size = len - offset;  // Adjust size if it goes beyond EOF

    size_t bytes_read = 0;
    size_t block_index = offset / IO_BLOCK_SIZE;  // Starting block index
    size_t block_offset = offset % IO_BLOCK_SIZE; // Offset within the first block
    // fprintf(stderr,"[%s ,%d] inode.metadata.size %d\n",__func__,__LINE__, inode.metadata.size);
    while (bytes_read < size && block_index < inode.metadata.size/IO_BLOCK_SIZE) {
        char block_buffer[IO_BLOCK_SIZE];  // Temporary buffer for each block
        fs->read(&inode, block_buffer, IO_BLOCK_SIZE, block_index*IO_BLOCK_SIZE);
        // fprintf(stderr,"[%s ,%d] block_index %d\n",__func__,__LINE__, block_index);
        size_t copy_size = std::min(size - bytes_read, IO_BLOCK_SIZE - block_offset);
        memcpy(buf + bytes_read, block_buffer + block_offset, copy_size);
        // fprintf(stderr,"[%s ,%d] buf %s, block_buffer %s\n",__func__,__LINE__, buf, block_buffer);
        bytes_read += copy_size;
        block_index++;
        block_offset = 0;  // Only the first block might have a non-zero offset
    }*/

    return bytes_read;  // Return the actual number of bytes read
}