#ifndef FS_CONSTANTS_HPP
#define FS_CONSTANTS_HPP

#include <algorithm>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define IO_BLOCK_SIZE 4096

#define NUM_INODE_BLOCKS 1023

#define INODE_SIZE 512

#define DATABLOCKS_PER_BITMAP_BLOCK 255

#endif