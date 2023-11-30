#include "fischl.h"
#include "fs.hpp"
#include <stdio.h>

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

  int err;

  // RawDisk *disk = new FakeRawDisk(2048);
  // Fs *fs = new Fs(disk);
  // fs->format();
  // disk->print_block(0);
  // disk->print_block(1);

  // INode_Data inode_data;
  // fs->inode_manager->new_inode(1, 2, 3, &inode_data);

  // disk->print_block(0);
  // disk->print_block(1);

  int BL_SIZE = 4096 / 8;

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

  u_int64_t big_buf[BL_SIZE * 1000];
  char *buf = (char *)big_buf;

  int offs = 55 * 4096;

  RawDisk *disk = new FakeRawDisk(2048);
  Fs *fs = new Fs(disk);

  fs->format();
  disk->print_block(0);
  disk->print_block(1);

  INode_Data inode_data;
  fs->inode_manager->new_inode(1, 2, 3, &inode_data);

  disk->print_block(0);
  disk->print_block(1);
  disk->print_block(1024);

  for (int i = 0; i < BL_SIZE * 3; ++i)
    big_buf[i] = 1;

  err = fs->write(&inode_data, buf, 4096 * 3, offs);

  for (int i = 0; i < BL_SIZE * 3; ++i)
    big_buf[i] = 2;

  err = fs->truncate(&inode_data, offs + 4096);
  err = fs->write(&inode_data, buf, 4096 * 2, offs + 4096 * 2);
  err = fs->truncate(&inode_data, offs + 4096 * 2);

  fs->inode_manager->save_inode(&inode_data);
  printf("Write %d", err);

  disk->print_block(0);
  disk->print_block(1);
  disk->print_block(1024);
  disk->print_block(1025);
  disk->print_block(1026);
  disk->print_block(1027);
  disk->print_block(1028);
  disk->print_block(1029);
  // disk->print_block(1080);
  // disk->print_block(1081);
  // disk->print_block(1082);
  // disk->print_block(1083);
  // disk->print_block(1084);
  // disk->print_block(1085);

  // err = fs->truncate(&inode_data, 4096 + 4);
  // fs->inode_manager->save_inode(&inode_data);
  // printf("Truncate %d", err);

  // disk->print_block(0);
  // disk->print_block(1);
  // disk->print_block(1024);
  // disk->print_block(1025);
  // disk->print_block(1026);
  // disk->print_block(1027);
  // disk->print_block(1028);

  err = fs->lseek_next_hole(&inode_data, offs + 0);
  printf("lseek_next_hole (%d): %d\n\n", offs + 0, err);
  err = fs->lseek_next_hole(&inode_data, offs + 1);
  printf("lseek_next_hole (%d): %d\n\n", offs + 1, err);
  err = fs->lseek_next_hole(&inode_data, offs + 4096);
  printf("lseek_next_hole (%d): %d\n\n", offs + 4096, err);
  err = fs->lseek_next_hole(&inode_data, offs + 4097);
  printf("lseek_next_hole (%d): %d\n\n", offs + 4097, err);
  err = fs->lseek_next_hole(&inode_data, offs + 8192);
  printf("lseek_next_hole (%d): %d\n\n", offs + 8192, err);
  err = fs->lseek_next_hole(&inode_data, offs + 8193);
  printf("lseek_next_hole (%d): %d\n\n", offs + 8193, err);
  err = fs->lseek_next_hole(&inode_data, offs + 12288);
  printf("lseek_next_hole (%d): %d\n\n", offs + 12288, err);
  err = fs->lseek_next_hole(&inode_data, offs + 12289);
  printf("lseek_next_hole (%d): %d\n\n", offs + 12289, err);
  err = fs->lseek_next_hole(&inode_data, offs + 100000);
  printf("lseek_next_hole (%d): %d\n\n", offs + 100000, err);

  return 0;
}