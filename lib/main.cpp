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

  int err;

  RawDisk *disk = new FakeRawDisk(2048);
  Fs *fs = new Fs(disk);
  fs->format();
  disk->print_block(0);
  disk->print_block(1);

  INode_Data inode_data;
  fs->inode_manager->new_inode(1, 2, 3, &inode_data);

  disk->print_block(0);
  disk->print_block(1);

  int BL_SIZE = 4096 / 8;

  u_int64_t buf[BL_SIZE * (56 + 512 + 10)];

  for (int i = 0; i < BL_SIZE * (56 + 512 + 10); ++i)
    buf[i] = (i / BL_SIZE) + 1;

  err = fs->write(&inode_data, (char *)buf, 4096 * (56 + 3) + 16 + 8, 0);
  fs->inode_manager->save_inode(&inode_data);

  printf("Write %d", err);

  disk->print_block(0);
  disk->print_block(1);
  disk->print_block(1025);
  disk->print_block(1026);
  disk->print_block(1027);
  disk->print_block(1080);
  disk->print_block(1081);
  disk->print_block(1082);
  disk->print_block(1083);
  disk->print_block(1084);
  disk->print_block(1085);

  int N = 5;

  u_int64_t buf2[4096] = {0};
  err = fs->read(&inode_data, (char *)buf2, (8 * N), 4096 - 8 - 8);

  printf("\n\nREAD: %d\n", err);
  for (int i = 0; i < N; ++i)
    printf("%d ", buf2[i]);
  printf("\n");

  return 0;
}