#include "disk.h"
#include "fs_specs.h"
#include "inode.h"
#include "block_layer.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int inode_bitmap_find_slot(block* bitmap);
void inode_bitmap_alloc(block* bitmap, int slot);
void inode_bitmap_free(block* bitmap, int slot);

/*
 * this function allocates an inode into the file system. it
 * finds an available slot and performs the writes to disk.
 *
 * returns the slot number if allocated, else -1.
 */
int inode_alloc(struct inode* inode) {
    // load inode bitmap block and find slot
    block bitmap_block;
    block_read(&bitmap_block, INODE_BITMAP_BLOCK);
    int slot = inode_bitmap_find_slot(&bitmap_block);

    // no free slots
    if (slot == -1) {
        return -1;
    }

    // load relevant inode table block
    int block_num = INODE_TABLE_START + (slot * INODE_SIZE) / BLOCK_SIZE;
    block table_block;
    block_read(&table_block, block_num);

    // write new inode to block
    int offset = INODE_SIZE * (slot % (NUM_INODES / INODE_TABLE_BLOCKS));
    memcpy(table_block, inode, sizeof(struct inode));

    // update inode bitmap
    inode_bitmap_alloc(&bitmap_block, slot);

    // write inode table and bitmap blocks back
    block_write(&bitmap_block, INODE_BITMAP_BLOCK);
    block_write(&table_block, block_num);

    // success
    return slot;
}

/*
 * this function frees an inode from the file system. it will clear
 * the inode entry from the bitmap and the inode table.
 *
 * returns the slot number in case of success, -1 otherwise.
 */
int inode_free(int slot) {
    // load inode bitmap block
    block bitmap_block;
    block_read(&bitmap_block, INODE_BITMAP_BLOCK);
    // update bitmap
    inode_bitmap_free(&bitmap_block, slot);
    // write back to disk
    block_write(&bitmap_block, INODE_BITMAP_BLOCK);
    return slot;
}

/*
 * reads the inode at a given slot from the inode table.
 */
int inode_read(struct inode* out, int slot) {
    // load in inode table block
    int block_num = INODE_TABLE_START + (slot * INODE_SIZE) / BLOCK_SIZE;
    block table_block;
    block_read(&table_block, block_num);

    // fetch inode in table block
    int offset = INODE_SIZE * (slot % (BLOCK_SIZE / INODE_SIZE));
    memcpy(&out, &table_block[offset], sizeof(struct inode));

    return 0;
}

/*
 * writes an inode struct to disk at a specified slot.
 */
int inode_write(struct inode* in, int slot) {
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
int inode_bitmap_find_slot(block* bitmap) {
    // ceil(n / k) = (n + k - 1) / k
    // k = 8 since 1 byte = 8 bits
    for (int i = 0; i < (NUM_INODES + 7) / 8; i++) {
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
 */
void inode_bitmap_alloc(block* bitmap, int slot) {
    int byte_ix = slot / 8;
    int bit_ix = slot % 8;

    if ((*bitmap)[byte_ix] & (1 << bit_ix)) {
        fprintf(stderr, "Warning: slot %d is already taken\n", slot);
        return;
    }

    (*bitmap)[byte_ix] |= (1 << bit_ix);
}

/*
 * modifies the inode bitmap to release an inode.
 */
void inode_bitmap_free(block* bitmap, int slot) {
    int byte_ix = slot / 8;
    int bit_ix = slot % 8;

    if (((*bitmap)[byte_ix] & (1 << bit_ix)) == 0) {
        fprintf(stderr, "Warning: slot %d is already free\n", slot);
        return;
    }

    (*bitmap)[byte_ix] &= ~(1 << bit_ix);
}
