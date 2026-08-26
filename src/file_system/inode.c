#include "inode.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../utils/utils.h"
#include "block_layer.h"
#include "file_system.h"

int inode_bitmap_find_available_slot(block* bitmap);
int inode_bitmap_alloc(block* bitmap, int slot);
int inode_bitmap_free(block* bitmap, int slot);

/*
 * this function allocates an inode into the file system. it
 * finds an available slot and performs the writes to disk. to
 * update an existing inode, refer to `inode_write()`.
 *
 * returns slot if allocated, else -1.
 */
int inode_alloc(struct inode* inode) {
    if (inode == NULL) {
        fprintf(stderr,
                "Warning: cannot allocate an inode whose pointer is NULL\n");
        return -1;
    }

    // load inode bitmap block
    block bitmap_block;
    block_read(&bitmap_block, INODE_BITMAP_BLOCK);

    // find empty slot in inode table
    int slot = inode_bitmap_find_available_slot(&bitmap_block);
    // no free slots
    if (slot == -1) {
        return -1;
    }

    // write inode entry in table
    if (inode_write(inode, slot) != 0) {
        return -1;
    }

    // update inode bitmap
    if (inode_bitmap_alloc(&bitmap_block, slot) != 0) {
        return -1;
    }

    // success
    return slot;
}

/*
 * this function frees an inode from the file system. it will clear
 * the inode entry from the bitmap and the inode table.
 *
 * returns slot in case of success, else -1.
 */
int inode_free(int slot) {
    if (slot < 0 || slot > NUM_INODES) {
        fprintf(stderr, "Warning: invalid inode slot number %d\n", slot);
    }

    // load inode bitmap block
    block bitmap_block;
    if (block_read(&bitmap_block, INODE_BITMAP_BLOCK) != 0) {
        return -1;
    }

    // update bitmap
    if (inode_bitmap_free(&bitmap_block, slot) != 0) {
        return -1;
    }

    return slot;
}

/*
 * reads the inode at a given slot from the inode table.
 *
 * returns 0 in case of success, else -1.
 */
int inode_read(struct inode* out, int slot) {
    if (out == NULL) {
        fprintf(stderr, "Warning: cannot read inode into a NULL buffer\n");
        return -1;
    }

    if (slot < 0 || slot > NUM_INODES) {
        fprintf(stderr, "Warning: invalid inode slot number %d\n", slot);
        return -1;
    }

    // load in inode table block
    int block_num = INODE_TABLE_START + (slot * INODE_SIZE) / BLOCK_SIZE;
    block table_block;
    block_read(&table_block, block_num);

    // fetch inode in table block
    int offset = INODE_SIZE * (slot % (BLOCK_SIZE / INODE_SIZE));
    memcpy(out, &table_block[offset], sizeof(struct inode));

    return 0;
}

/*
 * writes an inode struct to disk at a specified slot. this
 * function is meant to update an existing inode. refer to
 * `inode_alloc()` to write a new entry to the file system.
 *
 * returns 0 in case of success, else -1
 */
int inode_write(struct inode* in, int slot) {
    if (in == NULL) {
        fprintf(stderr, "Warning: cannot write NULL inode pointer to disk\n");
        return -1;
    }

    if (slot < 0 || slot > NUM_INODES) {
        fprintf(stderr, "Warning: invalid inode slot number %d\n", slot);
        return -1;
    }

    // load in inode table block
    int block_num = INODE_TABLE_START + (slot * INODE_SIZE) / BLOCK_SIZE;
    block table_block;
    block_read(&table_block, block_num);

    // perform update to table block
    int offset = INODE_SIZE * (slot % (BLOCK_SIZE / INODE_SIZE));
    memcpy(&table_block[offset], in, sizeof(struct inode));

    // write table block back to disk
    block_write(&table_block, block_num);

    return 0;
}

/*
 * this function linearly scans the inode bitmap until it finds
 * an available inode.
 *
 * returns the inode number if an available one is found. otherwise,
 * returns -1.
 */
int inode_bitmap_find_available_slot(block* bitmap) {
    // we want ceil(n, k) where n = num of inodes and k = 8 since 1 byte = 8
    // bits
    for (int i = 0; i < ceili(NUM_INODES, 8); i++) {
        uint8_t byte = (*bitmap)[i];

        for (int j = 0; j < 8; j++) {
            int slot = i * 8 + j;

            // out of bounds
            if (slot >= NUM_INODES) {
                return -1;
            }

            // bitwise AND to see if it is taken
            if (!(byte & (0x80 >> j))) {
                return slot;
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
int inode_bitmap_alloc(block* bitmap, int slot) {
    int byte_ix = slot / 8;
    int bit_ix = slot % 8;

    if ((*bitmap)[byte_ix] & (1 << bit_ix)) {
        fprintf(stderr, "Warning: slot %d is already taken\n", slot);
        return -1;
    }

    (*bitmap)[byte_ix] |= (1 << bit_ix);

    // write back to disk
    if (block_write(bitmap, INODE_BITMAP_BLOCK) != 0) {
        return -1;
    }

    return 0;
}

/*
 * modifies the inode bitmap to release an inode.
 *
 * returns 0 in case of success, else -1.
 */
int inode_bitmap_free(block* bitmap, int slot) {
    int byte_ix = slot / 8;
    int bit_ix = slot % 8;

    if (((*bitmap)[byte_ix] & (1 << bit_ix)) == 0) {
        fprintf(stderr, "Warning: slot %d is already free\n", slot);
        return -1;
    }

    (*bitmap)[byte_ix] &= ~(1 << bit_ix);

    // write back to disk
    if (block_write(bitmap, INODE_BITMAP_BLOCK) != 0) {
        return -1;
    }

    return 0;
}
