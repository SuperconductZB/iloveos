#include <sys/types.h>
#include <fs.hpp>
#include <fuse.h>
#include "direntry.h"

class FilesOperation {
    RawDisk& disk;
    Fs *fs;
    void create_dot_dotdot(INode_Data*, u_int64_t);

    public:
    TreeNode *root_node;
    FilesOperation(RawDisk&, Fs*);
    //int read_datablock(const INode_Data& inode, u_int64_t index, char* buffer);
    //int write_datablock(INode_Data& inode, u_int64_t index, char* buffer);
    void initialize_rootinode();
    void printbuffer(const char*,int);
    void printDirectory(u_int64_t);
    INode_Data* create_new_inode(u_int64_t parent_inode_number, const char* name, mode_t mode);
    int insert_inode_to(u_int64_t parent_inode_number, const char* name, INode_Data *new_inode);
    void unlink_inode(u_int64_t inode_number);
    u_int64_t disk_namei(const char* path);
    u_int64_t namei(const char* path);
    int fischl_mkdir(const char*, mode_t);
    int fischl_mknod(const char*, mode_t, dev_t);//for special file
    int fischl_create(const char *, mode_t, struct fuse_file_info *);//for regular file
    int fischl_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi);
    int fischl_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *, enum fuse_readdir_flags);
    int fischl_releasedir(const char* path, struct fuse_file_info *fi);
    int fischl_unlink (const char *);
    int fischl_rmdir(const char *);
    int fischl_readlink(const char* path, char* buf, size_t size);
    int fischl_symlink(const char* from, const char* to);
    int fischl_link(const char* from, const char* to);
    int fischl_rename(const char *path, const char *, unsigned int flags);
    int fischl_truncate(const char *path, off_t, struct fuse_file_info *fi);
    int fischl_chmod(const char *path, mode_t, struct fuse_file_info *fi);
    int fischl_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi);
    int fischl_open (const char *, struct fuse_file_info *);//open file
    int fischl_release (const char *, struct fuse_file_info *);//close file
    int fischl_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);
    int fischl_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
    int fischl_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi);
};