#pragma once

#include "file_system.h"

typedef struct extent {
    int logical_start;  // starting block of the file
    int data_start;     // starting data block on disk
    int block_count;    // how many blocks
} extent;

typedef struct inode {
    enum file_type file_type;
    int inum;     // stores inode number
    int size;             // size of file/dir content in bytes
    int blocks_occupied;  // num data block occupied
    int extent_count;
    extent extents[MAX_EXTENTS];
} inode;

int inode_alloc(struct inode* inode);
int inode_free(int inode);
int inode_read(struct inode* out, int inode);
int inode_write(struct inode* in, int inode);
