#include "fischl.h"
#include "fs.hpp"
#include <stdio.h>

int main() {
  //   printf("hello word!");
  //   fischl *F = new fischl;
  //   F->init();
  // char *d = strdup("/dev/vdc");
  RawDisk *disk = new FakeRawDisk(2048);
  Fs *fs = new Fs(disk);
  fs->format();
  disk->print_block(0);
  disk->print_block(1);
  INode_Data inode_data = INode_Data();
  fs->inode_allocator->new_inode(1, 2, 3, &inode_data);
  int err;
  for (int i = 0; i < 56 + 512 + 4; ++i)
    err = fs->allocate_datablock(&inode_data);

  for (int i = 0; i < 3; ++i)
    printf("%d\n", err = fs->deallocate_datablock(&inode_data));

  fs->save_inode(&inode_data);

  disk->print_block(0);
  disk->print_block(1);

  disk->print_block(1081);

  disk->print_block(1596);

  disk->print_block(1597);

  return 0;
}