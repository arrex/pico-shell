#pragma once

#include "inode.h"

// special metadata.. maybe i will come up w smth creative later
#define FS_TYPE "idk"

#define BLOCK_SIZE 1024  // 1 kB
#define NUM_BLOCKS 128

// -3 encompasses superblock, inode bitmap, and data bitmap
#define NUM_DATA_BLOCKS \
    (NUM_BLOCKS - (NUM_INODES * sizeof(struct inode)) / BLOCK_SIZE - 3)

// file system disk layout
#define SUPERLOCK_BLOCK 0
#define INODE_BITMAP_BLOCK 1
#define DATA_BITMAP_BLOCK 2
#define INODE_TABLE_START 3
#define INODE_TABLE_BLOCKS 5
#define DATA_REGION_START 8

#define MAX_FILENAME_LEN 59
#define MAX_EXTENTS 4

enum file_type;

// file system components
typedef struct superblock {
    char fs_type[sizeof(FS_TYPE)];
    int num_inodes;
    // stores which disk block marks the start of the inodes table
    int inodes_table_start;
    int num_data_blocks;
    int block_size;  // in bytes
} superblock;

int fs_init();
int fs_chdir(char* path);
int fs_create(char* path, enum file_type type);
int fs_delete(char* path, enum file_type type);
// TODO: implement primitives below
void fs_read();
void fs_write();
