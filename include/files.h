#include <sys/types.h>
#include <fs.hpp>
#include "fuse_common.h"
#include "direntry.h"

class FilesOperation {
    RawDisk& disk;
    Fs *fs;
    void create_dot_dotdot(INode_Data*, u_int64_t);

    public:
    TreeNode *root_node;
    FilesOperation(RawDisk&, Fs*);
    int read_datablock(const INode_Data& inode, u_int64_t index, char* buffer);
    int write_datablock(INode_Data& inode, u_int64_t index, char* buffer);
    void initialize_rootinode();
    void printDirectory(u_int64_t);
    INode_Data* create_new_inode(u_int64_t parent_inode_number, const char* name, mode_t mode);
    void unlink_inode(u_int64_t inode_number);
    u_int64_t disk_namei(const char* path);
    u_int64_t namei(const char* path);
    int fischl_mkdir(const char*, mode_t);
    int fischl_mknod(const char*, mode_t, dev_t);//for special file
    int fischl_create(const char *, mode_t, struct fuse_file_info *);//for regular file
    //int fischl_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *, enum fuse_readdir_flags);
    int fischl_unlink (const char *);
    int fischl_open (const char *, struct fuse_file_info *);//open file
    int fischl_release (const char *, struct fuse_file_info *);//close file
    int fischl_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);
    int fischl_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
};