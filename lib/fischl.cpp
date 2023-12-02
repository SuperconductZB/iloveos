#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include "fs.hpp"
#include "files.h"

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
    RawDisk *H; // Use FakeRawDisk here if memory sanitizer complains
    Fs *fs;
    FilesOperation *fsop;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

void* fischl_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    options.fsop->initialize_rootinode();
}

int fischl_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	return options.fsop->fischl_create(path, mode, fi);
}


void fischl_destroy(void* private_data) {

}

static int fischl_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    return options.fsop->fischl_getattr(path, stbuf, fi);
}

static int fischl_access(const char* path, int mask) {

    // return 0 when access is allowed
    return 0;
} 

static int fischl_readlink(const char* path, char* buf, size_t size) {
    return -1;
}

static int fischl_opendir(const char* path, struct fuse_file_info* fi) {
    u_int64_t fh = options.fsop->namei(path);
    fi->fh = fh;
    return 0;
}

static int fischl_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t ft, struct fuse_file_info *fi, enum fuse_readdir_flags flg) {
    return options.fsop->fischl_readdir(path, buf, filler, ft, fi, flg);
}

static int fischl_mknod(const char* path, mode_t mode, dev_t rdev) {
    return options.fsop->fischl_mknod(path, mode, rdev);
}

static int fischl_mkdir(const char *path, mode_t mode) {
    return options.fsop->fischl_mkdir(path, mode);
}

static int fischl_unlink(const char* path) {
	return options.fsop->fischl_unlink(path);
}

static int fischl_rmdir(const char* path) {
    return options.fsop->fischl_rmdir(path);
}

static int fischl_symlink(const char* to, const char* from) {
    return -1;
}

static int fischl_rename(const char *path, const char *new_name, unsigned int flags) {
    return options.fsop->fischl_rename(path, new_name, flags);
}

static int fischl_link(const char* from, const char* to) {
    return -1;
}

static int fischl_chmod(const char *path, mode_t mode, struct fuse_file_info *fi) {
    return options.fsop->fischl_chmod(path, mode, fi);
}

static int fischl_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi) {
    return options.fsop->fischl_chown(path, uid, gid, fi);
}

static int fischl_truncate(const char *path, off_t offset, struct fuse_file_info *fi) {
    return options.fsop->fischl_truncate(path, offset, fi);
}

static int fischl_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi) {
    return -1;
}

static int fischl_open(const char *path, struct fuse_file_info *fi) {
    return options.fsop->fischl_open(path, fi);
}

static int fischl_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi) {
	return options.fsop->fischl_read(path, buf, size, offset, fi);
}

static int fischl_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    return options.fsop->fischl_write(path, buf, size, offset, fi);
}

static int fischl_statfs(const char* path, struct statvfs* stbuf) {
    return -1;
}

static int fischl_release(const char* path, struct fuse_file_info *fi) {
	return options.fsop->fischl_release(path, fi);
}

static int fischl_releasedir(const char* path, struct fuse_file_info *fi) {
    return options.fsop->fischl_releasedir(path, fi);
}


static const struct fuse_operations fischl_oper = {
	
    
    .getattr     = fischl_getattr,
    //.readlink    = fischl_readlink,
    .mknod       = fischl_mknod,
    .mkdir       = fischl_mkdir,
    .unlink      = fischl_unlink,
    .rmdir       = fischl_rmdir,
    //.symlink     = fischl_symlink,
    .rename      = fischl_rename,
    //.link        = fischl_link,
    .chmod       = fischl_chmod,
    .chown       = fischl_chown,
    .truncate    = fischl_truncate,
    .open        = fischl_open,
    .read        = fischl_read,
    .write       = fischl_write,
    //.statfs      = fischl_statfs,
    .release     = fischl_release,
    /*
#ifdef HAVE_SETXATTR
    .setxattr    = fischl_setxattr,
    .getxattr    = fischl_getxattr,
    .listxattr   = fischl_listxattr,
    .removexattr = fischl_removexattr,
#endif
*/
    .opendir     = fischl_opendir,
    .readdir     = fischl_readdir,
    .releasedir  = fischl_releasedir,
    .init        = fischl_init,
    .destroy     = fischl_destroy,
    .access      = fischl_access,
    .create      = fischl_create,
    //.utimens     = fischl_utimens,
    //.bmap        = fischl_bmap,
    //.ioctl       = fischl_ioctl,
    //.poll        = fischl_poll,
};

static void show_help(const char *progname)
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
    srand(time(NULL)); // Seed the random number generator
    //const char* d = (argc < 2) ? "/dev/vdc" : argv[1];

    //setupTestDirectory(&options.root);
    options.H = new FakeRawDisk(23552);
    options.fs = new Fs(options.H);
    options.fs->format();
    options.fsop = new FilesOperation(*options.H, options.fs);



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