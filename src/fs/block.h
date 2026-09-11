#include <stdlib.h>

#include "fs.h"

typedef uint8_t block[BLOCK_SIZE];

int block_read(block* buf, int bnum);
int block_write(const block* buf, int bnum);
