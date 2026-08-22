#include "file_system.h"

typedef struct dir_entry {
    char filename[MAX_FILENAME_LEN];
    int inode;
} dir_entry;

int data_block_alloc();
int data_block_free(int data_block);
