#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

void* fischl_init(struct fuse_conn_info *conn) {

}

int fischl_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	return FilesOperation::fischl_create(path, mode, fi);
}


void fischl_destroy(void* private_data) {

}

static int fischl_getattr(const char* path, struct stat* stbuf) {

	return 0;
}

static int fischl_fgetattr(const char* path, struct stat* stbuf) {

	return 0;
}

static int fischl_access(const char* path, mask) {

}

static int fischl_readlink(const char* path, char* buf, size_t size) {

}

static int fischl_opendir(const char* path, struct fuse_file_info* fi) {

}

static int fischl_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {

	return 0;
}

static int fischl_mknod(const char* path, mode_t mode, dev_t rdev) {

}

static int fischl_mkdir(const char *path, mode_t mode) {
    return FilesOperation::fischl_mkdir(path, mode);
}

static int fischl_unlink(const char* path) {
	return FilesOperation::fischl_unlink(path);
}

static int fischl_rmdir(const char* path) {

}

static int fischl_symlink(const char* to, const char* from) {

}

static int fischl_rename(const char* from, const char* to) {

}

static int fischl_link(const char* from, const char* to) {

}

static int fischl_chmod(const char* path, mode_t mode) {

}

static int fischl_chown(const char* path, uid_t uid, gid_t gid) {

}

static int fischl_truncate(const char* path, off_t size) {

}

static int fischl_ftruncate(const char* path, off_t size) {

}

static int fischl_utimens(const char* path, const struct timespec ts[2]) {

}

static int fischl_open(const char *path, struct fuse_file_info *fi) {
    return FilesOperation::fischl_open(path, fi);
}

static int fischl_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi) {
	return FilesOperation::fischl_read(path, buf, size, offset, fi);
}

static int fischl_write(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi) {
	return FilesOperation::fischl_write(path, buf, size, offset, fi);
}

static int fischl_statfs(const char* path, struct statvfs* stbuf) {

}

static int fischl_release(const char* path, struct fuse_file_info *fi) {
	return FilesOperation::fischl_release(path, fi);
}

static int fischl_releasedir(const char* path, struct fuse_file_info *fi) {

}

static int fischl_bmap(const char* path, size_t blocksize, uint64_t* blockno) {

}

static int fischl_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi, unsigned int flags, void* data) {

}

static int fischl_poll(const char* path, struct fuse_file_info* fi, struct fuse_pollhandle* ph, unsigned* reventsp){

}


static const struct fuse_operations fischl_oper = {
	.init        = fischl_init,
    .destroy     = fischl_destroy,
    .getattr     = fischl_getattr,
    .fgetattr    = fischl_fgetattr,
    .access      = fischl_access,
    .readlink    = fischl_readlink,
    .readdir     = fischl_readdir,
    .mknod       = fischl_mknod,
    .mkdir       = fischl_mkdir,
    .symlink     = fischl_symlink,
    .unlink      = fischl_unlink,
    .rmdir       = fischl_rmdir,
    .rename      = fischl_rename,
    .link        = fischl_link,
    .chmod       = fischl_chmod,
    .chown       = fischl_chown,
    .truncate    = fischl_truncate,
    .ftruncate   = fischl_ftruncate,
    .utimens     = fischl_utimens,
    .create      = fischl_create,
    .open        = fischl_open,
    .read        = fischl_read,
    .write       = fischl_write,
    .statfs      = fischl_statfs,
    .release     = fischl_release,
    .opendir     = fischl_opendir,
    .releasedir  = fischl_releasedir,
    .bmap        = fischl_bmap,
    .ioctl       = fischl_ioctl,
    .poll        = fischl_poll,
#ifdef HAVE_SETXATTR
    .setxattr    = fischl_setxattr,
    .getxattr    = fischl_getxattr,
    .listxattr   = fischl_listxattr,
    .removexattr = fischl_removexattr,
#endif
    .flag_nullpath_ok = 0,
};

static void fischl::show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"fischl\" file\n"
	       "                        (default: \"fischl\")\n"
	       "    --contents=<s>      Contents \"fischl\" file\n"
	       "                        (default \"fischl, World!\\n\")\n"
	       "\n");
}

int fischl(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);



	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}

	ret = fuse_main(args.argc, args.argv, &fischl_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}