#include "data.h"

#include <stdio.h>

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

/*
 * this function linearly scans the data bitmap until it finds
 * an available data block.
 *
 * returns the data block number if an available one is found. otherwise,
 * returns -1.
 */
static int data_bitmap_find_free() {
    block bitmap;
    block_read(&bitmap, DATA_BITMAP_BLOCK);

    // ceil(n / k) = (n + k - 1) / k
    // k = 8 since 1 byte = 8 bits
    for (int i = 0; i < (NUM_DATA_BLOCKS + 7) / 8; i++) {
        uint8_t byte = bitmap[i];

        for (int j = 0; j < 8; j++) {
            int data_block = i * 8 + j;

            // out of bounds
            if (data_block >= NUM_DATA_BLOCKS) {
                return -1;
            }

            // bitwise AND to see if it is taken
            if (!(byte & (0x80 >> j))) {
                return data_block;
            }
        }
    }

    // no data block available
    return -1;
}

/*
 * modifies the data bitmap to claim a data block.
 *
 * returns 0 on success, -1 on failure.
 */
int data_bitmap_alloc(int dnum) {
    block bitmap;
    block_read(&bitmap, DATA_BITMAP_BLOCK);

    int byte_ix = dnum / 8;
    int bit_ix = dnum % 8;

    if (bitmap[byte_ix] & (0x80 >> bit_ix)) {
        return -1;
    }

    bitmap[byte_ix] |= (0x80 >> bit_ix);

    // write data bitmap block back
    if (block_write(&bitmap, DATA_BITMAP_BLOCK) != 0) {
        return -1;
    }

    return 0;
}

/*
 * modifies the data bitmap to release a data block.
 *
 * returns 0 on success, -1 on failure.
 */
int data_bitmap_free(int dnum) {
    block bitmap;
    block_read(&bitmap, DATA_BITMAP_BLOCK);

    int byte_ix = dnum / 8;
    int bit_ix = dnum % 8;

    if ((bitmap[byte_ix] & (0x80 >> bit_ix)) == 0) {
        return -1;
    }

    bitmap[byte_ix] &= ~(0x80 >> bit_ix);

    // write data bitmap block back
    if (block_write(&bitmap, DATA_BITMAP_BLOCK) != 0) {
        return -1;
    }

    return 0;
}
