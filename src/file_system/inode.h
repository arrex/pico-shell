#include "file_system.h"

typedef struct extent {
    int logical_start;
    int physical_start;
    int block_count;
} extent;

typedef struct inode {
    enum file_type file_type;
    int parent; // stores parent inode number
    int size; // size of file/dir content in bytes
    int blocks_occupied; // num data block occupied
    extent extents[MAX_EXTENTS];
} inode;

int inode_alloc(struct inode* inode);
int inode_free(int slot);
int inode_read(struct inode* out, int slot);
int inode_write(struct inode* in, int slot);
