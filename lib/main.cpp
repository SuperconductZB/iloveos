#define _GNU_SOURCE

#include "fischl.h"
#include "fs.hpp"
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

int main() {
  //   printf("hello word!");
  //   fischl *F = new fischl;
  //   F->init();
  // char *d = strdup("/dev/vdc");

  // RawDisk *disk = new FakeRawDisk(2048);
  // Fs *fs = new Fs(disk);
  // fs->format();
  // disk->print_block(0);
  // disk->print_block(1);
  // INode_Data inode_data = INode_Data();
  // fs->inode_manager->new_inode(1, 2, 3, &inode_data);
  // int err;
  // u_int64_t block_num = 0;
  // for (int i = 0; i < 56 + 512 + 4; ++i)
  //   err = fs->allocate_datablock(&inode_data, &block_num);

  // for (int i = 0; i < 5; ++i)
  //   printf("%d\n", err = fs->deallocate_datablock(&inode_data, &block_num));

  // fs->inode_manager->save_inode(&inode_data);

  // disk->print_block(0);
  // disk->print_block(1);

  // disk->print_block(1081);

  // disk->print_block(1596);

  // disk->print_block(1597);

  // return 0;

  // int err;

  // RawDisk *disk = new FakeRawDisk(2048);
  // Fs *fs = new Fs(disk);
  // fs->format();
  // disk->print_block(0);
  // disk->print_block(1);

  // INode_Data inode_data;
  // fs->inode_manager->new_inode(1, 2, 3, &inode_data);

  // disk->print_block(0);
  // disk->print_block(1);

  // int BL_SIZE = 4096 / 8;

  // u_int64_t buf[BL_SIZE * (56 + 512 + 10)];

  // for (int i = 0; i < BL_SIZE * (56 + 512 + 10); ++i)
  //   buf[i] = (i / BL_SIZE) + 1;

  // err = fs->write(&inode_data, (char *)buf, 4096 * (56 + 3) + 16 + 8, 0);
  // fs->inode_manager->save_inode(&inode_data);

  // printf("Write %d", err);

  // disk->print_block(0);
  // disk->print_block(1);
  // disk->print_block(1025);
  // disk->print_block(1026);
  // disk->print_block(1027);
  // disk->print_block(1080);
  // disk->print_block(1081);
  // disk->print_block(1082);
  // disk->print_block(1083);
  // disk->print_block(1084);
  // disk->print_block(1085);

  // int N = 5;

  // u_int64_t buf2[4096] = {0};
  // err = fs->read(&inode_data, (char *)buf2, (8 * N), 4096 - 8 - 8);

  // printf("\n\nREAD: %d\n", err);
  // for (int i = 0; i < N; ++i)
  //   printf("%d ", buf2[i]);
  // printf("\n");

  // u_int64_t big_buf[BL_SIZE * 1000];
  // char *buf = (char *)big_buf;

  // int offs = 55 * 4096;

  // RawDisk *disk = new FakeRawDisk(2048);
  // Fs *fs = new Fs(disk);

  // fs->format();
  // disk->print_block(0);
  // disk->print_block(1);

  // INode_Data inode_data;
  // fs->inode_manager->new_inode(1, 2, 3, &inode_data);

  // disk->print_block(0);
  // disk->print_block(1);
  // disk->print_block(1024);

  // for (int i = 0; i < BL_SIZE * 3; ++i)
  //   big_buf[i] = 1;

  // err = fs->write(&inode_data, buf, 4096 * 3, offs);

  // for (int i = 0; i < BL_SIZE * 3; ++i)
  //   big_buf[i] = 2;

  // err = fs->truncate(&inode_data, offs + 4096);
  // err = fs->write(&inode_data, buf, 4096 * 2, offs + 4096 * 2);
  // err = fs->truncate(&inode_data, offs + 4096 * 2);

  // fs->inode_manager->save_inode(&inode_data);
  // printf("Write %d", err);

  // disk->print_block(0);
  // disk->print_block(1);
  // disk->print_block(1024);
  // disk->print_block(1025);
  // disk->print_block(1026);
  // disk->print_block(1027);
  // disk->print_block(1028);
  // disk->print_block(1029);
  // // disk->print_block(1080);
  // // disk->print_block(1081);
  // // disk->print_block(1082);
  // // disk->print_block(1083);
  // // disk->print_block(1084);
  // // disk->print_block(1085);

  // // err = fs->truncate(&inode_data, 4096 + 4);
  // // fs->inode_manager->save_inode(&inode_data);
  // // printf("Truncate %d", err);

  // // disk->print_block(0);
  // // disk->print_block(1);
  // // disk->print_block(1024);
  // // disk->print_block(1025);
  // // disk->print_block(1026);
  // // disk->print_block(1027);
  // // disk->print_block(1028);

  // err = fs->lseek_next_hole(&inode_data, offs + 0);
  // printf("lseek_next_hole (%d): %d\n\n", offs + 0, err);
  // err = fs->lseek_next_hole(&inode_data, offs + 1);
  // printf("lseek_next_hole (%d): %d\n\n", offs + 1, err);
  // err = fs->lseek_next_hole(&inode_data, offs + 4096);
  // printf("lseek_next_hole (%d): %d\n\n", offs + 4096, err);
  // err = fs->lseek_next_hole(&inode_data, offs + 4097);
  // printf("lseek_next_hole (%d): %d\n\n", offs + 4097, err);
  // err = fs->lseek_next_hole(&inode_data, offs + 8192);
  // printf("lseek_next_hole (%d): %d\n\n", offs + 8192, err);
  // err = fs->lseek_next_hole(&inode_data, offs + 8193);
  // printf("lseek_next_hole (%d): %d\n\n", offs + 8193, err);
  // err = fs->lseek_next_hole(&inode_data, offs + 12288);
  // printf("lseek_next_hole (%d): %d\n\n", offs + 12288, err);
  // err = fs->lseek_next_hole(&inode_data, offs + 12289);
  // printf("lseek_next_hole (%d): %d\n\n", offs + 12289, err);
  // err = fs->lseek_next_hole(&inode_data, offs + 100000);
  // printf("lseek_next_hole (%d): %d\n\n", offs + 100000, err);

  // RawDisk *disk = new FakeRawDisk(2048);
  // Fs *fs = new Fs(disk);
  // fs->format();

  // INode_Data inode_data;
  // fs->inode_manager->new_inode(1, 2, 3, &inode_data);

  // char cwd_buf[PATH_MAX];
  // int fd;

  // assert(getcwd(cwd_buf, sizeof(cwd_buf)) != NULL);

  // printf("\n\n(");

  // printf(cwd_buf);

  // printf(")\n\n");

  // fd = open("/home/connor", O_TMPFILE | O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
  // assert(fd != -1);

  // u_int64_t test_start_range = IO_BLOCK_SIZE * 100;
  // u_int64_t test_write_range = IO_BLOCK_SIZE * 50;
  // u_int64_t test_read_range = IO_BLOCK_SIZE * 50;

  // char zeros[test_write_range] = {0};
  // char ones[test_write_range];
  // memset(ones, 1, test_write_range);

  // char *write_buf = ones;
  // char reference_read_buf[test_read_range];
  // char test_read_buf[test_read_range];
  // size_t offset, count;
  // int test_res, ref_res;

  // for (int i = 0; i < 1000; ++i) {
  //   offset = rand() & test_start_range;

  //   switch (rand() % 2) {
  //   case 0:
  //     count = rand() & test_write_range;
  //     write_buf = (write_buf == ones) ? zeros : ones;
  //     printf("write: %ds count=%d offset=%d\n", write_buf[0], count, offset);
  //     test_res = fs->write(&inode_data, write_buf, count, offset);
  //     assert(lseek(fd, offset, SEEK_SET) == offset);
  //     ref_res = write(fd, write_buf, count);
  //     break;
  //   case 1:
  //     count = rand() & test_read_range;
  //     printf("read: count=%d offset=%d\n", count, offset);
  //     test_res = fs->read(&inode_data, test_read_buf, count, offset);
  //     assert(lseek(fd, offset, SEEK_SET) == offset);
  //     ref_res = read(fd, reference_read_buf, count);
  //     bool reads_are_equal = true;
  //     for (size_t j = 0; j < count; ++j)
  //       if (test_read_buf[i] != reference_read_buf[i]) {
  //         reads_are_equal = false;
  //         break;
  //       }
  //     assert(reads_are_equal);
  //     break;
  //   }

  //   assert(test_res == ref_res);
  // }

  RawDisk *disk = new FakeRawDisk(5120);
  Fs *fs = new Fs(disk);
  fs->format();

  int buf_size = IO_BLOCK_SIZE * 200;
  int loops = 14 * 1024 * 1024 / buf_size;

  char buf[buf_size];

  memset(buf, 1, sizeof(buf));

  INode_Data inode_data;
  fs->inode_manager->new_inode(1, 2, 3, &inode_data);

  int res;

  for (int j = 0; j < loops; ++j) {
    res = fs->write(&inode_data, buf, sizeof(buf), sizeof(buf) * j);
    printf("write: %d j=%d\n", res, j);
  }

  for (int j = 0; j < loops; ++j) {

    memset(buf, 0, sizeof(buf));
    res = fs->read(&inode_data, buf, sizeof(buf), sizeof(buf) * j);

    printf("read: %d j=%d\n", res, j);

    for (int i = 0; i < sizeof(buf); ++i)
      if (buf[1] != 1) {
        printf("error:  %d\n", i);
        return -1;
      }
  }

  return 0;
}