#include "block_layer.h"

#include <stdio.h>

int data_bitmap_find_available_block(block* bitmap);
void data_bitmap_alloc(block* bitmap, int block_num);
void data_bitmap_free(block* bitmap, int block_num);

/*
 * this function allocates a data block into the file system. it
 * finds an available data block and performs the writes to disk.
 *
 * returns the block number if allocated, else -1.
 */
int data_block_alloc(block* buf) {
    // load data bitmap block and find block num
    block bitmap_block;
    block_read(&bitmap_block, DATA_BITMAP_BLOCK);
    int block_num = data_bitmap_find_available_block(&bitmap_block);

    // no free block numbers
    if (block_num == -1) {
        return -1;
    }

    // update bitmap
    data_bitmap_alloc(&bitmap_block, block_num);

    // write data bitmap block back
    block_write(&bitmap_block, DATA_BITMAP_BLOCK);

    // success
    return block_num;
}

/*
 * this function frees a data block from the file system.
 * it will clear the data block entry from the bitmap.
 *
 * returns the block number in case of success, -1 otherwise.
 */
int data_block_free(int block_num) {
    // load data bitmap block
    block bitmap_block;
    block_read(&bitmap_block, DATA_BITMAP_BLOCK);

    // update bitmap
    data_bitmap_free(&bitmap_block, block_num);

    // write back to disk
    block_write(&bitmap_block, block_num);

    return block_num;
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
            int block_num = i * 8 + j;

            // out of bounds
            if (block_num >= NUM_DATA_BLOCKS) {
                return -1;
            }

            // bitwise AND to see if it is taken
            if (!(byte & (0x80 >> j))) {
                return block_num;
            }
        }
    }

    // no data block available
    return -1;
}

/*
 * modifies the data bitmap to claim a data block.
 */
void data_bitmap_alloc(block* bitmap, int block_num) {
    int byte_ix = block_num / 8;
    int bit_ix = block_num % 8;

    if ((*bitmap)[byte_ix] & (1 << bit_ix)) {
        fprintf(stderr, "Warning: block number %d is already taken\n", block_num);
        return;
    }

    (*bitmap)[byte_ix] |= (1 << bit_ix);
}

/*
 * modifies the data bitmap to release a data block.
 */
void data_bitmap_free(block* bitmap, int block_num) {
    int byte_ix = block_num / 8;
    int bit_ix = block_num % 8;

    if (((*bitmap)[byte_ix] & (1 << bit_ix)) == 0) {
        fprintf(stderr, "Warning: block number %d is already free\n", block_num);
        return;
    }

    (*bitmap)[byte_ix] &= ~(1 << bit_ix);
}
