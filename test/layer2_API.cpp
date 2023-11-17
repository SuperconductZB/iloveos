#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "files.h"

int main(int argc, char *argv[]) {
    const char* d = (argc < 2) ? "/dev/vdc" : argv[1];
    
    RawDisk *H = new RawDisk(d);

    printf("test files\n");
    FilesOperation fsop(*H);
    fsop.initialize_rootinode();
    
    // create multiple files using mkdir or mknod
    // directories that contain more than 64 files use more than one datablocks, it is not supported yet
    printf("=== Part 1: create files by path ===\n");
    u_int64_t file1 = fsop.fischl_mknod("/test",0); // mode here is not used yet
    printf("/test is inode %llu, it is a file\n", file1);
    u_int64_t file2 = fsop.fischl_mkdir("/foo",0);
    printf("/foo is inode %llu, it is a directory\n", file2);
    u_int64_t file3 = fsop.fischl_mkdir("/foo/bar",0);
    printf("/foo/bar is inode %llu, it is a directory\n", file3);
    u_int64_t file4 = fsop.fischl_mknod("/foo/bar/baz",0);
    printf("/foo/bar/baz is inode %llu, it is a file\n", file4);
    // the following three testcases will fail
    u_int64_t f1 = fsop.fischl_mkdir("foo/bar",0);
    u_int64_t f2 = fsop.fischl_mkdir("/doesnt_exist/bar",0);
    u_int64_t f3 = fsop.fischl_mkdir("/test/bar",0);
    // TODO: guard against creating an existing file or diretory, such as fsop.fischl_mkdir("/test",0)

    // write to files (TODO: fischl_write)
    // read and write to indirect datablocks are not supported yet
    printf("=== Part 2: write to files ===\n");
    char buffer[IO_BLOCK_SIZE] = {0};
    INode inode;
    inode.inode_construct(file1, *H);
    buffer[0] = '1';
    fsop.write_datablock(inode, 0, buffer);
    inode.inode_save(*H);
    inode.inode_construct(file4, *H);
    buffer[0] = '4';
    fsop.write_datablock(inode, 3, buffer);
    inode.inode_save(*H);
    // TODO: guard against overwriting directory datablocks

    // retrieve inode-number by path
    printf("=== Part 3: retrieve inode number using path (namei) ===\n");
    u_int64_t file_test = fsop.namei("/test");
    printf("inode number for \"/test\" is %llu\n", file_test);
    assert(file_test == file1);
    u_int64_t file_baz = fsop.namei("/foo/bar/baz");
    printf("inode number for \"/foo/bar/baz\" is %llu\n", file_baz);
    assert(file_baz == file4);

    // read files (TODO: fischl_read)
    printf("=== Part 4: read from files ===\n");
    char read_buffer[IO_BLOCK_SIZE] = {0};
    INode inode_read;
    inode_read.inode_construct(file_test, *H);
    fsop.read_datablock(inode_read, 0, read_buffer);
    assert(read_buffer[0] == '1');
    inode_read.inode_construct(file_baz, *H);
    fsop.read_datablock(inode_read, 3, read_buffer);
    assert(read_buffer[0] == '4');
}