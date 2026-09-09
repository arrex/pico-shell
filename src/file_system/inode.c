#include "inode.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/utils.h"
#include "block.h"
#include "file_system.h"

static int inode_bitmap_find_free();
static int inode_bitmap_alloc(int inum);
static int inode_bitmap_free(int inum);

/*
 * this function allocates an inode into the file system. it
 * finds an available slot and performs the writes to disk.
 * it also mutates the inum of the input struct.
 *
 * to update an existing inode, refer to `inode_update()`.
 *
 * returns 0 if allocated, else -1.
 */
int inode_alloc(struct inode* inode) {
    if (inode == NULL) {
        fprintf(
            stderr,
            "[inode] error: cannot allocate an inode whose pointer is NULL\n");
        return -1;
    }

    int inum;
    if ((inum = inode_bitmap_find_free()) == -1) {
        return -1;
    }
    inode->inum = inum;

    // write inode to table
    int bnum = IBLOCK(inum);
    block block;
    if (block_read(&block, bnum) != 0) {
        return -1;
    }

    int offset = IOFFSET(inum);
    memcpy(&block[offset], inode, sizeof(struct inode));
    if (block_write(&block, bnum) != 0) {
        return -1;
    }

    // update inode bitmap
    if (inode_bitmap_alloc(inum) != 0) {
        return -1;
    }

    return 0;
}

/*
 * this function frees an inode from the file system by modifying the bitmap
 * only. we do not clear the actual entry on disk to save an I/O.
 *
 * returns 0 in case of success, else -1.
 */
int inode_free(int inum) {
    if (inum < 0 || inum > NUM_INODES) {
        fprintf(stderr, "[inode] error: invalid inode number %d\n", inum);
    }

    if (inode_bitmap_free(inum) != 0) {
        return -1;
    }

    return 0;
}

/*
 * reads the inode at a given slot from the inode table.
 *
 * returns 0 in case of success, else -1.
 */
int inode_read(struct inode* inode, int inum) {
    if (inode == NULL) {
        fprintf(stderr,
                "[inode] error: cannot read inode into a NULL buffer\n");
        return -1;
    }

    if (inum < 0 || inum >= NUM_INODES) {
        fprintf(stderr, "[inode] error: invalid inode number %d\n", inum);
        return -1;
    }

    int bnum = IBLOCK(inum);
    block block;
    if (block_read(&block, bnum) != 0) {
        return -1;
    }

    int offset = IOFFSET(inum);
    memcpy(inode, &block[offset], sizeof(struct inode));

    return 0;
}

/*
 * writes an inode struct to disk at a specified slot. this
 * function is meant to update an existing inode. refer to
 * `inode_alloc()` to write a new entry to the file system.
 *
 * returns 0 in case of success, else -1
 */
int inode_update(const struct inode* inode, int inum) {
    if (inode == NULL) {
        fprintf(stderr,
                "[inode] error: cannot write NULL inode pointer to disk\n");
        return -1;
    }

    if (inum < 0 || inum >= NUM_INODES) {
        fprintf(stderr, "[inode] error: invalid inode number %d\n", inum);
        return -1;
    }

    int bnum = IBLOCK(inum);
    block block;
    if (block_read(&block, bnum) != 0) {
        return -1;
    }

    int offset = IOFFSET(inum);
    memcpy(&block[offset], inode, sizeof(struct inode));
    if (block_write(&block, bnum) != 0) {
        return -1;
    }

    return 0;
}

/*
 * this function linearly scans the inode bitmap until it finds
 * an available inode.
 *
 * returns the inode number if an available one is found. otherwise,
 * returns -1.
 */
static int inode_bitmap_find_free() {
    block bitmap;
    block_read(&bitmap, INODE_BITMAP_BLOCK);

    // we want ceil(n / k) where n = num of inodes and k = 8 since 1 byte = 8
    // bits
    for (int i = 0; i < ceili(NUM_INODES, 8); i++) {
        uint8_t byte = bitmap[i];

        for (int j = 0; j < 8; j++) {
            int inum = i * 8 + j;

            // out of bounds
            if (inum >= NUM_INODES) {
                return -1;
            }

            // bitwise AND to see if it is taken
            if (!(byte & (0x80 >> j))) {
                return inum;
            }
        }
    }

    // no inode available
    return -1;
}

/*
 * modifies the inode bitmap to claim an inode.
 *
 * returns 0 in case of success, else -1.
 */
static int inode_bitmap_alloc(int inum) {
    block bitmap;
    block_read(&bitmap, INODE_BITMAP_BLOCK);

    int byte_ix = inum / 8;
    int bit_ix = inum % 8;

    if (bitmap[byte_ix] & (0x80 >> bit_ix)) {
        fprintf(stderr, "[inode] error: inode number %d is already taken\n",
                inum);
        return -1;
    }

    bitmap[byte_ix] |= (0x80 >> bit_ix);

    // write back to disk
    if (block_write(&bitmap, INODE_BITMAP_BLOCK) != 0) {
        return -1;
    }

    return 0;
}

/*
 * modifies the inode bitmap to release an inode.
 *
 * returns 0 in case of success, else -1.
 */
static int inode_bitmap_free(int inum) {
    block bitmap;
    block_read(&bitmap, INODE_BITMAP_BLOCK);

    int byte_ix = inum / 8;
    int bit_ix = inum % 8;

    if ((bitmap[byte_ix] & (0x80 >> bit_ix)) == 0) {
        fprintf(stderr, "[inode] error: inode number %d is already free\n",
                inum);
        return -1;
    }

    bitmap[byte_ix] &= ~(0x80 >> bit_ix);

    // write back to disk
    if (block_write(&bitmap, INODE_BITMAP_BLOCK) != 0) {
        return -1;
    }

    return 0;
}
