#include "fs_specs.h"

typedef struct dir_entry {
    char filename[MAX_FILENAME_LEN];
    int inode;
} dir_entry;

int data_block_alloc(struct block* buf);
int data_block_free(int block_num);
