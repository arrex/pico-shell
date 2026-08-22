#include "block_layer.h"

#include <stdio.h>

int data_bitmap_find_available_block(block* bitmap);
int data_bitmap_alloc(block* bitmap, int data_block);
int data_bitmap_free(block* bitmap, int data_block);

/*
 * this function allocates a data block into the file system. it
 * finds an available data block and performs the writes to disk.
 *
 * returns data block number if allocated, else -1.
 */
int data_block_alloc() {
    // load data bitmap block and find block num
    block bitmap_block;
    block_read(&bitmap_block, DATA_BITMAP_BLOCK);
    int data_block = data_bitmap_find_available_block(&bitmap_block);

    // no free block numbers
    if (data_block == -1) {
        return -1;
    }

    // update bitmap
    int errCode = data_bitmap_alloc(&bitmap_block, data_block);
    if (errCode != 0) {
        return -1;
    }

    // write data bitmap block back
    block_write(&bitmap_block, DATA_BITMAP_BLOCK);

    // success
    return data_block;
}

/*
 * this function frees a data block from the file system.
 * it will clear the data block entry from the bitmap.
 *
 * returns data block number in case of success, else -1.
 */
int data_block_free(int data_block) {
    // load data bitmap block
    block bitmap_block;
    block_read(&bitmap_block, DATA_BITMAP_BLOCK);

    // update bitmap
    int errCode = data_bitmap_free(&bitmap_block, data_block);
    if (errCode != 0) {
        return -1;
    }

    // write back to disk
    block_write(&bitmap_block, data_block);

    return data_block;
}

/*
 * this function linearly scans the data bitmap until it finds
 * an available data block.
 *
 * returns the data block number if an available one is found. otherwise,
 * returns -1.
 */
int data_bitmap_find_available_block(block* bitmap) {
    // ceil(n / k) = (n + k - 1) / k
    // k = 8 since 1 byte = 8 bits
    for (int i = 0; i < (NUM_DATA_BLOCKS + 7) / 8; i++) {
        uint8_t byte = (*bitmap)[i];

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
int data_bitmap_alloc(block* bitmap, int data_block) {
    int byte_ix = data_block / 8;
    int bit_ix = data_block % 8;

    if ((*bitmap)[byte_ix] & (1 << bit_ix)) {
        fprintf(stderr, "Warning: block number %d is already taken\n", data_block);
        return -1;
    }

    (*bitmap)[byte_ix] |= (1 << bit_ix);

    return 0;
}

/*
 * modifies the data bitmap to release a data block.
 *
 * returns 0 on success, -1 on failure.
 */
int data_bitmap_free(block* bitmap, int data_block) {
    int byte_ix = data_block / 8;
    int bit_ix = data_block % 8;

    if (((*bitmap)[byte_ix] & (1 << bit_ix)) == 0) {
        fprintf(stderr, "Warning: block number %d is already free\n", data_block);
        return -1;
    }

    (*bitmap)[byte_ix] &= ~(1 << bit_ix);

    return 0;
}
