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

static int fischl_getattr(const char* path, struct stat* stbuf) {

	return 0;
}

static int fischl_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {

	return 0;
}

static int fischl_mkdir(const char *, mode_t) {

    return 0;
}

static int fischl_open(const char *path, struct fuse_file_info *fi) {
    
    return 0;
}

static int fischl_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi) {
	
	return 0;
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
    .fsync       = fischl_fsync,
    .flush       = fischl_flush,
    .fsyncdir    = fischl_fsyncdir,
    .lock        = fischl_lock,
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