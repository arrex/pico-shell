#include "file_system.h"

#include <stdlib.h>

typedef uint8_t block[BLOCK_SIZE];

void block_read(block* buf, int block_num);
void block_write(const block* buf, int block_num);
