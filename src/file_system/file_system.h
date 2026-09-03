#pragma once

// our own extent-based fs type
#define FS_TYPE "my_ext"

#define BLOCK_SIZE 2048  // 2kB
#define NUM_BLOCKS 64

#define INODE_SIZE 128  // 128B
#define NUM_INODES 80   // 16 inodes/block across 5 blocks
#define ROOT_INODE 0

// -3 encompasses superblock, inode bitmap, and data bitmap
#define NUM_DATA_BLOCKS \
    (NUM_BLOCKS - (NUM_INODES * INODE_SIZE) / BLOCK_SIZE - 3)

// file system disk layout
#define SUPERLOCK_BLOCK 0
#define INODE_BITMAP_BLOCK 1
#define DATA_BITMAP_BLOCK 2
#define INODE_TABLE_START 3
#define INODE_TABLE_BLOCKS 5
#define DATA_REGION_START 8

#define MAX_FILENAME_LEN 59
#define MAX_EXTENTS 4

enum file_type { UNUSED_T, FILE_T, DIRECTORY_T };

// file system components
typedef struct superblock {
    char file_system_type[sizeof(FS_TYPE)];
    int num_inodes;
    // stores which disk block marks the start of the inodes table
    int inodes_table_start;
    int num_data_blocks;
    int block_size;  // in bytes
} superblock;

int fs_init();
int fs_create(char* path, enum file_type type);
void fs_delete();
void fs_read();
void fs_write();
