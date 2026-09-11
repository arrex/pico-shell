#include "../types/types.h"
#include "fs.h"

#define MAX_FILE_SIZE (NDIRECT * BLOCK_SIZE)

struct inode;

int file_read(struct inode*, char*, int, int);
int file_write(struct inode*, char*, uint, uint);
