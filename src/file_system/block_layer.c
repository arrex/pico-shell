#include "block_layer.h"

#include <stdio.h>

#include "disk.h"
#include "file_system.h"

/*
 * reads a specified block on disk.
 *
 * returns 0 in case of success, else -1.
 */
int block_read(block* buf, int block_num) {
    if (buf == NULL) {
        fprintf(stderr, "Warning: input buffer is null\n");
        return -1;
    }

    if (block_num < 0 || block_num >= NUM_BLOCKS) {
        fprintf(stderr, "Warning: invalid block number %d\n", block_num);
    }

    return disk_read(buf, BLOCK_SIZE, block_num * BLOCK_SIZE);
}

/*
 * writes to a specified block on disk.
 *
 * returns 0 in case of success, else -1.
 */
int block_write(const block* buf, int block_num) {
    if (buf == NULL) {
        fprintf(stderr, "Warning: input buffer is null\n");
        return -1;
    }

    if (block_num < 0 || block_num >= NUM_BLOCKS) {
        fprintf(stderr, "Warning: invalid block number %d\n", block_num);
    }

    return disk_write(buf, BLOCK_SIZE, block_num * BLOCK_SIZE);
}
