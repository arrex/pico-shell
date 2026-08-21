#include "file_system.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// our own grossly simplified abstraction of the hardware disk
uint8_t disk[NUM_BLOCKS * BLOCK_SIZE] = {0};

void superblock_init();
void inode_table_init();
int get_block_base_address(int block);
void write_disk(int base_address, const void* data, int size);

void fs_init() {
    // init fs components
    superblock_init(); // block 0
    // block 1: inode bitmap
    // block 2: data bitmap
    inode_table_init(); // next couple of blocks
    // rest of blocks belong to data region
}

void superblock_init() {
    struct superblock superblock = {
        .file_system_type = FS_TYPE,
        .num_inodes = NUM_INODES,
        .inodes_table_start = 3,
        // -3 encompasses superblock, inode bitmap, and data bitmap
        .num_data_blocks = NUM_BLOCKS - ((NUM_INODES * INODE_SIZE) / BLOCK_SIZE) - 3,
        .block_size = BLOCK_SIZE
    };

    // write to first block
    int base_address = get_block_base_address(0);
    write_disk(0, &superblock, sizeof(struct superblock));
}

void inode_table_init() {
    struct inode inode_table[NUM_INODES];
    for (int i = 0; i < NUM_INODES; i++) {
        struct inode inode = {
            .file_type = UNUSED_T,
            .size = INODE_SIZE,
            .blocks_occupied = 0,
            // init w sentinels
            .direct = {-1},
            .indirect = -1
        };

        inode_table[i] = inode;
    }

    int inode_table_blocks = (NUM_INODES * INODE_SIZE) / BLOCK_SIZE;
    for (int block = 0; block < inode_table_blocks; block++) {
        // first 3 blocks for superblock and bitmaps
        int base_address = get_block_base_address(3 + block);
        int inode_ix = block * (BLOCK_SIZE / INODE_SIZE);
        write_disk(base_address, &inode_table[inode_ix], BLOCK_SIZE);
    }
}

int get_block_base_address(int block) {
    return block * BLOCK_SIZE;
}

void write_disk(int base_address, const void* data, int size) {
    if (size > BLOCK_SIZE) {
        printf("Warning: size %d exceeds block size\n", size);
    }

    memcpy(&disk[base_address], data, size);
}
