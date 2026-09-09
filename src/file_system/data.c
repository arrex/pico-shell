#include "data.h"

#include <stdio.h>

#include "bitmap.h"
#include "block.h"
#include "file_system.h"

static int data_bitmap_find_free();
static int data_bitmap_alloc(int dnum);
static int data_bitmap_free(int dnum);

/*
 * this function allocates a data block into the file system. it
 * finds an available data block from the bitmap and performs the
 * write to disk.
 *
 * returns data block number if allocated, else -1.
 */
int data_alloc() {
    int dnum;
    if ((dnum = data_bitmap_find_free()) == -1) {
        return -1;
    }

    // write to disk
    int bnum = DBLOCK(dnum);
    block block = {0};
    if (block_write(&block, bnum) != 0) {
        return -1;
    }

    // update data bitmap
    if (data_bitmap_alloc(dnum) != 0) {
        fprintf(stderr, "[data] error: data block number %d is already taken\n",
                dnum);
        return -1;
    }

    return dnum;
}

/*
 * this function frees a data block from the file system by clearing the data
 * block entry from the bitmap. it does not wipe what the block contains on
 * disk.
 *
 * returns 0 in case of success, else -1.
 */
int data_free(int dnum) {
    if (dnum < 0 || dnum >= NUM_DATA_BLOCKS) {
        fprintf(stderr, "[data] error: data block %d is out of bounds\n", dnum);
        return -1;
    }

    if (data_bitmap_free(dnum) != 0) {
        fprintf(stderr, "[data] error: data block number %d is already free\n",
                dnum);
        return -1;
    }

    return 0;
}

/*
 * reads the specified data block into a buffer.
 *
 * returns 0 in case of success, else -1.
 */
int data_read(block* block, int dnum) {
    if (block == NULL) {
        fprintf(stderr,
                "[data] error: cannot read data block into a NULL buffer\n");
        return -1;
    }

    if (dnum < 0 || dnum >= NUM_DATA_BLOCKS) {
        fprintf(stderr, "[data] error: data block %d is out of bounds\n", dnum);
        return -1;
    }

    int bnum = DBLOCK(dnum);
    if (block_read(block, bnum) != 0) {
        return -1;
    }

    return 0;
}

/*
 * writes buffer to a specified data block on disk. ensure that the
 * data block being written to has been allocated.
 *
 * returns 0 in case of success, else -1.
 */
int data_update(block* block, int dnum) {
    if (block == NULL) {
        fprintf(stderr, "[data] error: cannot write NULL data block to disk\n");
        return -1;
    }

    if (dnum < 0 || dnum >= NUM_DATA_BLOCKS) {
        fprintf(stderr, "[data] error: data block %d is out of bounds\n", dnum);
        return -1;
    }

    int bnum = DBLOCK(dnum);
    if (block_write(block, bnum) != 0) {
        return -1;
    }

    return 0;
}

static int data_bitmap_find_free() {
    return bitmap_find_free(DATA_BITMAP_BLOCK, NUM_DATA_BLOCKS);
}

int data_bitmap_alloc(int dnum) {
    return bitmap_alloc(DATA_BITMAP_BLOCK, dnum);
}

int data_bitmap_free(int dnum) { return bitmap_free(DATA_BITMAP_BLOCK, dnum); }
