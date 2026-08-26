#include "file_system.h"

#include <stdlib.h>

typedef uint8_t block[BLOCK_SIZE];

int block_read(block* buf, int block_num);
int block_write(const block* buf, int block_num);
