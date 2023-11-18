#include <sys/types.h>
#include <fs.h>

class FilesOperation {
    RawDisk& disk;
    INodeOperation inop;
    u_int64_t root_inode;
    public:
    FilesOperation(RawDisk&);
    int read_datablock(INode& inode, u_int64_t index, char* buffer);
    int write_datablock(INode& inode, u_int64_t index, char* buffer);
    INode* new_inode(u_int64_t inode_number, u_int64_t permissions);
    void initialize_rootinode();
    u_int64_t mkfile(u_int64_t parent_inode_number, const char* name, u_int64_t permissions);
    u_int64_t namei(const char* path);
    u_int64_t fischl_mkdir(const char*, mode_t);
    u_int64_t fischl_mknod(const char*, mode_t);
    //int fischl_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *, enum fuse_readdir_flags);
    //int fischl_unlink (const char *);
    //int fischl_open (const char *, struct fuse_file_info *);
};