#include <sys/types.h>

int fischl_mkdir(const char*, mode_t);
int fischl_mknod(const char*, mode_t);
int fischl_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *, enum fuse_readdir_flags);
int fischl_unlink (const char *);
int fischl_open (const char *, struct fuse_file_info *);