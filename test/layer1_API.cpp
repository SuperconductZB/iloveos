#include "fs.hpp"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  // const char* d = (argc < 2) ? "/dev/vdc" : argv[1];

  RawDisk *H = new FakeRawDisk(2048);
  Fs *fs = new Fs(H);

  printf("test inode\n");
  fs->format();
  char buffer[IO_BLOCK_SIZE] = {0};
  /**************************test inode
   * Initialization***************************/
  // test the begining of inode 1th
  H->read_block(1, buffer);
  u_int64_t t = 0;
  for (int j = 0; j < 8; j++)
    t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);

  assert(t ==
         2); // the first 1th unused inode will store the next unused inode 2th

  assert(IO_BLOCK_SIZE == 4096);
  assert(INODE_SIZE == 512);

  // test the number before end of inode
  H->read_block(NUM_INODE_BLOCKS, buffer);
  t = 0;
  for (int j = 0; j < 8; j++)
    t |=
        ((u_int64_t)(unsigned char)buffer[j + IO_BLOCK_SIZE - (INODE_SIZE * 2)])
        << (8 * j);

  assert(t == NUM_INODE_BLOCKS *
                  (IO_BLOCK_SIZE / INODE_SIZE)); // store the maximun th inode

  // test the end of inode
  t = 0;
  for (int j = 0; j < 8; j++)
    t |= ((u_int64_t)(unsigned char)buffer[j + IO_BLOCK_SIZE - INODE_SIZE])
         << (8 * j);

  assert(
      t ==
      0); // the end of inode(524287th inode) do not have the next inode address
  /**************************test datablock
   * Initialization***************************/
  // we separate 2048 4kB I/O block(1+2047) as a group and the first I/O block
  // will manage the following 2047 I/O block usage. the first 8 bytes(0~7) in
  // first I/O block store the address of next first I/O block, the following
  // 256(8~263) bytes record 2047 I/O block usage. test the begining of free
  // datablock
  H->read_block(NUM_INODE_BLOCKS + 1,
                buffer); // the begining of free datablock will store
                         // from address (MAX_INODE) * SECTOR_SIZE
  t = 0;
  for (int j = 0; j < 8; j++)
    t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);

  assert(t == NUM_INODE_BLOCKS + 1 + DATABLOCKS_PER_BITMAP_BLOCK +
                  1); // the first 8 bytes of 4k I/O block will store
                      // the next address(after 2048*4k I/O block)
  // test the end of the datablock
  // H->read_block(NUM_BLOCKS - DATABLOCKS_PER_BITMAP_BLOCK - 1, buffer);
  // t = 0;
  // for (int j = 0; j < 8; j++)
  //   t |= ((u_int64_t)(unsigned char)buffer[j]) << (8 * j);

  // assert(t == NUM_BLOCKS - DATABLOCKS_PER_BITMAP_BLOCK - 1);

  /***************************test inode
   * de/allocation**********************************/
  // when requesting an inode, the inode_allocation will give you the inode
  // number, we use inode_list to store the sequence allocate inode arrary
  // version
  INode_Data inode_list[20];
  u_int64_t record_free[10] = {
      0}; // should do a pop up structure to record the free inode
  int rec = 9;
  // printf("Allocate 20 inode num:{");
  for (int i = 0; i < 20; i++) {
    fs->inode_manager->new_inode(0, 0, 0, &inode_list[i]);

    assert(inode_list[i].inode_num == i + 1);
    // printf(" %d", inode_list[i].inode_num);
  }
  // printf("}\n");
  for (int i = 10; i < 20; i++) {
    record_free[i - 10] = inode_list[i].inode_num;
    fs->inode_manager->free_inode(
        &inode_list[i]); // free the 10 element from inode_list[10]
  }
  // allocate again the last 10
  printf("Allocate again: inode num:{");
  for (int i = 10; i < 20; i++) {
    fs->inode_manager->new_inode(0, 0, 0, &inode_list[i]);
    // printf("inode %d, rec_f %d\n,", inode_list[i],record_free[rec]);
    assert(inode_list[i].inode_num == record_free[rec]);
    rec--;
  }
  printf("}\n");
  /***************************test direct blocks[48]
   * de/allocation**********************************/
  // after free the datablock, the program will find the first smallest address
  // of datablock to give to the inode should test random resize each node, but
  // should use datablock_free data structure to record
  //   u_int64_t rec_datablock_free[10][3] = {0}; // array version
  //   u_int64_t temp_block_num = 0;
  //   for (int i = 0; i < 10; i++) {
  //     // printf("%dth data block starting addres: ", i);
  //     for (int j = 0; j < 6; j++) {
  //       fs->allocate_datablock(&inode_list[i], &temp_block_num);
  //       // printf("%d," ,inode_inside[i].datablock_allocate(*H));
  //     }
  //     // printf("\n");
  //   }
  //   for (int i = 0; i < 10; i++) {
  //     // printf("%dth data block free addres: ", i);
  //     for (int j = 2; j >= 0; j--) {
  //       fs->deallocate_datablock(&inode_list[i],
  //       &(rec_datablock_free[i][j]));
  //       // printf("", rec_datablock_free[i][j]);
  //     }
  //     // printf("\n");
  //   }

  //   for (int i = 0; i < 10; i++) {
  //     // printf("%dth data block allocate again addres: ", i);
  //     for (int j = 0; j < 3; j++) {
  //       fs->allocate_datablock(&inode_list[i], &temp_block_num);
  //       assert(temp_block_num == rec_datablock_free[i][j]);
  //       // printf("%d," ,inode_inside[i].datablock_allocate(*H));
  //     }
  //     // printf("\n");
  //   }

  // printf("}\n");
  delete H; // Delete the RawDisk object

  return 0;
}
