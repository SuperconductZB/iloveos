// #include "fs.h"
// #include <assert.h>
// #include <inttypes.h>
// #include <stdio.h>
// #include <string.h>

// // in fs.h:
// // #define MAX_INODE 2048
// // #define MAX_BLOCKNUM 51200
// // 51200 Sectors = 2048 * 25 Sectors = 25MB
// // Free List Heads: 2048*512, 2048*9*512, 2048*17*512
// // Available INodes: 2047 (-1 because superblock)
// // Available DataBlocks (including Indirect Blocks): 2047 * 3 = 6141

// int main(int argc, char *argv[]) {
//   const char *d = (argc < 2) ? "/dev/vdc" : argv[1];

//   RawDisk *H = new FakeRawDisk(d);

//   printf("=== INode Alloc/Dealloc Test ===\n");
//   INodeOperation inop;
//   inop.initialize(*H);

//   // Test INode alloc and dealloc
//   int inode_list[2046] = {
//       0}; // if we allocate 2047 inodes head will be 0 (faulty)
//   printf("freeInodeHead: %d \n",
//          SuperBlock::getFreeINodeHead(*H)); // this impl should give 1
//   for (int i = 0; i < 2046; i++) {
//     inode_list[i] = inop.inode_allocate(*H);
//     if (SuperBlock::getFreeINodeHead(*H) == 0) {
//       printf("%d\n", i);
//       assert(false);
//     }
//   }
//   printf("freeInodeHead: %d \n",
//          SuperBlock::getFreeINodeHead(*H)); // this impl should give 2047
//   for (int i = 0; i < 1024; i++) {
//     inop.inode_free(*H, inode_list[i]);
//   }
//   for (int i = 0; i < 1022; i++) {
//     inode_list[i] = inop.inode_allocate(*H);
//     assert(SuperBlock::getFreeINodeHead(*H) != 0);
//   }
//   printf("freeInodeHead: %d \n",
//          SuperBlock::getFreeINodeHead(*H)); // this impl should give 2
//   inode_list[1022] = inop.inode_allocate(*H);
//   printf("freeInodeHead: %d \n",
//          SuperBlock::getFreeINodeHead(*H)); // this impl should give 1
//   inode_list[1023] = inop.inode_allocate(*H);
//   printf("freeInodeHead: %d \n",
//          SuperBlock::getFreeINodeHead(*H)); // this impl should give 2047

//   // Test Many Files
//   printf("=== Many Files Test ===\n");
//   INode inode_inside[100];
//   for (int i = 0; i < 100; i++) {
//     inode_inside[i].inode_construct(inode_list[i], *H);
//     for (int j = 0; j < 60;
//          j++) { // Note that 1 indirect block is used for each file
//       u_int64_t allcBlockNum = inode_inside[i].datablock_allocate(*H);
//       if (SuperBlock::getFreeListHead(*H) >= (u_int64_t)51200 * 512) {
//         printf("Bad FreeListHead: %d, %d, %llu\n", i, j,
//                SuperBlock::getFreeListHead(*H));
//         assert(false);
//       }
//       if (allcBlockNum % 2048 != 0 || allcBlockNum < 2048 * 512 ||
//           allcBlockNum >= 25 * 2048 * 512) {
//         printf("Bad Allocated Block Number: %d, %d, %llu\n", i, j,
//                allcBlockNum);
//         assert(false);
//       }
//     }
//   }
//   printf("Finished Allocating\n");
//   // in this impl should give 17*2048*512 = 17825792
//   printf(
//       "freeListHead: %llu \n",
//       SuperBlock::getFreeListHead(
//           *H)); // if all 6141 blocks allocated, would give 51200*512
//           (faulty)
//   for (int i = 0; i < 100; i++) {
//     for (int j = 0; j < 59; j++) {
//       u_int64_t freedBlkNum = inode_inside[i].datablock_deallocate(*H);
//       u_int64_t fh = SuperBlock::getFreeListHead(*H);
//       if (freedBlkNum % 2048 != 0 || freedBlkNum < 2048 * 512 ||
//           freedBlkNum >= 25 * 2048 * 512 || fh >= 51200 * 512) {
//         printf("%d, %d, Freed Block Number: %llu\n", i, j, freedBlkNum);
//         printf("FreeListHead is %llu\n", fh);
//         assert(false);
//       }
//     }
//   }
//   printf("Finished Deallocating\n");
//   printf("freeListHead: %d \n", SuperBlock::getFreeListHead(*H));

//   // Test Big File (Use direct, single indirect, double indirect)
//   printf("=== Big File Test ===\n");
//   u_int64_t lastAllc = 0;
//   for (int j = 0; j < 5000; j++) {
//     u_int64_t allcBlockNum = inode_inside[0].datablock_allocate(*H);
//     lastAllc = allcBlockNum;
//     u_int64_t fh = SuperBlock::getFreeListHead(*H);
//     if (allcBlockNum % 2048 != 0 || allcBlockNum < 2048 * 512 ||
//         allcBlockNum >= 25 * 2048 * 512 || fh >= 51200 * 512) {
//       printf("%d, Alloc Block Number: %llu\n", j, allcBlockNum);
//       printf("FreeListHead is %llu\n", fh);
//       assert(false);
//     }
//   }
//   printf("last allocate for big file: %llu\n", lastAllc);
//   printf("Finished Allocating\n");
//   printf("freeListHead: %d \n", SuperBlock::getFreeListHead(*H));
//   for (int j = 0; j < 5000; j++) {
//     u_int64_t freedBlkNum = inode_inside[0].datablock_deallocate(*H);
//     u_int64_t fh = SuperBlock::getFreeListHead(*H);
//     if (freedBlkNum % 2048 != 0 || freedBlkNum < 2048 * 512 ||
//         freedBlkNum >= 25 * 2048 * 512 || fh >= 51200 * 512) {
//       printf("%d, Freed Block Number: %llu\n", j, freedBlkNum);
//       printf("FreeListHead is %llu\n", fh);
//       assert(false);
//     }
//   }
//   printf("Finished Deallocating\n");
//   printf("freeListHead: %d \n", SuperBlock::getFreeListHead(*H));

//   delete H; // Delete the RawDisk object

//   return 0;
// }
