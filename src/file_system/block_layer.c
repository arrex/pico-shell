#include "disk.h"
#include "block_layer.h"

void block_read(block* buf, int block_num) {
    disk_read(buf, BLOCK_SIZE, block_num * BLOCK_SIZE);
}

void block_write(const block* buf, int block_num) {
    disk_write(buf, BLOCK_SIZE, block_num * BLOCK_SIZE);
}
