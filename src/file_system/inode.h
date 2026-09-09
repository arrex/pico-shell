#pragma once

#include "file_system.h"

#define ROOT_INODE 0
#define NUM_INODES 80  // 16 inodes/block w 5 blocks
#define NDIRECT 12     // num of direct ptrs

#define INODES_PER_BLOCK (BLOCK_SIZE / sizeof(struct inode))
#define IBLOCK(inum)           \
    (inum / INODES_PER_BLOCK + \
     INODE_TABLE_START)  // calculates which block inode is on
#define IOFFSET(inum)            \
    ((inum % INODES_PER_BLOCK) * \
     sizeof(struct inode))  // calculates offset of inode in block

enum file_type { UNUSED_T, FILE_T, DIRECTORY_T };

typedef struct inode {
    enum file_type type;  // file or dir
    int inum;             // stores inode number
    int size;             // size of file/dir content in bytes
    int num_links;        // number of directory entries referring to this inode
    int addrs[NDIRECT];   // direct block ptrs
} inode;

int inode_alloc(struct inode* inode);
int inode_free(int inum);
int inode_read(struct inode* inode, int inum);
int inode_update(const struct inode* inode, int inum);
