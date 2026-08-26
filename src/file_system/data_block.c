#include "data_block.h"

#include <stdio.h>

#include "block_layer.h"
#include "file_system.h"

int data_bitmap_find_available_block();
int data_bitmap_alloc(int data_block);
int data_bitmap_free(int data_block);

/*
 * this function allocates a data block into the file system. it
 * finds an available data block and performs the writes to disk.
 *
 * returns data block number if allocated, else -1.
 */
int data_block_alloc() {
    int data_block = data_bitmap_find_available_block();
    // no free block numbers
    if (data_block == -1) {
        return -1;
    }

    // update bitmap
    if (data_bitmap_alloc(data_block) != 0) {
        return -1;
    }

    // zero out data block when claiming it
    block init = {0};
    if (data_block_write(&init, data_block) != 0) {
        return -1;
    }

    return data_block;
}

/*
 * this function frees a data block from the file system.
 * it will clear the data block entry from the bitmap.
 *
 * returns data block number in case of success, else -1.
 */
int data_block_free(int data_block) {
    if (data_block < 0 || data_block >= NUM_DATA_BLOCKS) {
        fprintf(stderr, "Warning: data block %d is out of bounds\n",
                data_block);
        return -1;
    }

    // update bitmap
    if (data_bitmap_free(data_block) != 0) {
        return -1;
    }

    return data_block;
}

/*
 * reads the specified data block into a buffer.
 *
 * returns 0 in case of success, else -1.
 */
int data_block_read(block* out, int data_block) {
    if (out == NULL) {
        fprintf(stderr, "Warning: cannot read data block into a NULL buffer\n");
        return -1;
    }

    if (data_block < 0 || data_block >= NUM_DATA_BLOCKS) {
        fprintf(stderr, "Warning: data block %d is out of bounds\n",
                data_block);
        return -1;
    }

    int physical_block = DATA_REGION_START + data_block;
    if (block_read(out, physical_block) != 0) {
        return -1;
    }

    return 0;
}

/*
 * writes buffer to a specified data block on disk.
 *
 * returns 0 in case of success, else -1.
 */
int data_block_write(block* in, int data_block) {
    if (in == NULL) {
        fprintf(stderr, "Warning: cannot write NULL data block to disk\n");
        return -1;
    }

    if (data_block < 0 || data_block >= NUM_DATA_BLOCKS) {
        fprintf(stderr, "Warning: data block %d is out of bounds\n",
                data_block);
        return -1;
    }

    int physical_block = DATA_REGION_START + data_block;
    if (block_write(in, physical_block) != 0) {
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
int data_bitmap_find_available_block() {
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
int data_bitmap_alloc(int data_block) {
    block bitmap;
    block_read(&bitmap, DATA_BITMAP_BLOCK);

    int byte_ix = data_block / 8;
    int bit_ix = data_block % 8;

    if (bitmap[byte_ix] & (0x80 >> bit_ix)) {
        fprintf(stderr, "Warning: block number %d is already taken\n",
                data_block);
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
int data_bitmap_free(int data_block) {
    block bitmap;
    block_read(&bitmap, DATA_BITMAP_BLOCK);

    int byte_ix = data_block / 8;
    int bit_ix = data_block % 8;

    if ((bitmap[byte_ix] & (0x80 >> bit_ix)) == 0) {
        fprintf(stderr, "Warning: block number %d is already free\n",
                data_block);
        return -1;
    }

    bitmap[byte_ix] &= ~(0x80 >> bit_ix);

    // write data bitmap block back
    if (block_write(&bitmap, DATA_BITMAP_BLOCK) != 0) {
        return -1;
    }

    return 0;
}
