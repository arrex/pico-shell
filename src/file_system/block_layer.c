#include "block_layer.h"

#include <stdio.h>

#include "disk.h"
#include "file_system.h"

/*
 * reads a specified block on disk.
 *
 * returns 0 in case of success, else -1.
 */
int block_read(block* buf, int bnum) {
    if (buf == NULL) {
        fprintf(stderr, "[block] error: input buffer is null\n");
        return -1;
    }

    if (bnum < 0 || bnum >= NUM_BLOCKS) {
        fprintf(stderr, "[block] error: invalid block number %d\n", bnum);
        return -1;
    }

    return disk_read(buf, BLOCK_SIZE, bnum * BLOCK_SIZE);
}

/*
 * writes to a specified block on disk.
 *
 * returns 0 in case of success, else -1.
 */
int block_write(const block* buf, int bnum) {
    if (buf == NULL) {
        fprintf(stderr, "[block] error: input buffer is null\n");
        return -1;
    }

    if (bnum < 0 || bnum >= NUM_BLOCKS) {
        fprintf(stderr, "[block] error: invalid block number %d\n", bnum);
        return -1;
    }

    return disk_write(buf, BLOCK_SIZE, bnum * BLOCK_SIZE);
}
