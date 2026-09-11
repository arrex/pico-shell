#include "block.h"

/*
 * this function reads the specified block from disk and treats it like a
 * bitmap. it will linearly scan the bitmap until it finds an available slot.
 * the limit arg specifies an exclusive upper bound (not included in range)
 * after which we stop searching.
 *
 * returns the position of the bit if an available slot is found, otherwise -1.
 */
int bitmap_find_free(int bnum, int limit) {
    block bitmap;
    if (block_read(&bitmap, bnum) != 0) {
        return -1;
    }

    // ceil(n / k) = (n + k - 1) / k
    // k = 8 since 1 byte = 8 bits
    for (int i = 0; i < (NUM_DATA_BLOCKS + 7) / 8; i++) {
        uint8_t byte = bitmap[i];

        for (int j = 0; j < 8; j++) {
            int pos = i * 8 + j;

            // out of bounds
            if (pos >= limit) {
                return -1;
            }

            // bitwise AND to see if it is taken
            if (!(byte & (0x80 >> j))) {
                return pos;
            }
        }
    }

    // no data block available
    return -1;
}

/*
 * reads in the specified block from disk and treats it like a bitmap. flips the
 * bir at the offest to 1 if it is 0.
 *
 * returns 0 on success, else -1.
 */
int bitmap_alloc(int bnum, int offset) {
    block bitmap;
    if (block_read(&bitmap, bnum) != 0) {
        return -1;
    }

    int byte_ix = offset / 8;
    int bit_ix = offset % 8;

    // already claimed
    if (bitmap[byte_ix] & (0x80 >> bit_ix)) {
        return -1;
    }

    // set bit to 1
    bitmap[byte_ix] |= (0x80 >> bit_ix);

    if (block_write(&bitmap, bnum) != 0) {
        return -1;
    }

    return 0;
}

/*
 * reads in the specified block from disk and treats it like a bitmap. flips the
 * bir at the offest to 0 if it is 1.
 *
 * returns 0 on success, else -1.
 */
int bitmap_free(int bnum, int offset) {
    block bitmap;
    if (block_read(&bitmap, bnum) != 0) {
        return -1;
    }

    int byte_ix = offset / 8;
    int bit_ix = offset % 8;

    // already free
    if ((bitmap[byte_ix] & (0x80 >> bit_ix)) == 0) {
        return -1;
    }

    // set bit to 0
    bitmap[byte_ix] &= ~(0x80 >> bit_ix);

    if (block_write(&bitmap, bnum) != 0) {
        return -1;
    }

    return 0;
}
