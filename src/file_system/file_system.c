#include "file_system.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block_layer.h"
#include "data_block.h"
#include "directory.h"
#include "disk.h"
#include "inode.h"

int superblock_init();
int root_dir_init();

/*
 * initializes the entire file system by writing the superblock and the
 * root directory entry.
 *
 * returns 0 in case of success, else -1.
 */
int fs_init() {
    int errCode;

    errCode = superblock_init();
    if (errCode != 0) {
        fprintf(stderr, "Warning: could not initialize superblock\n");
        return -1;
    }

    errCode = root_dir_init();
    if (errCode != 0) {
        fprintf(stderr, "Warning: could not initialize root directory\n");
        return -1;
    }

    return 0;
}

int fs_create(const char* path, enum file_type type) {
    // read root inode to see where data block is
    uint8_t buffer[sizeof(struct inode)];
    int base_address = INODE_TABLE_START * BLOCK_SIZE + ROOT_INODE * INODE_SIZE;
    disk_read(buffer, base_address, sizeof(struct inode));

    // read root inode's data block to see entries
    // recursively do the same until we hit end of path
    // if already exists -> error
    // find available inode from bitmap
    // if no inode available -> error
    // find available data block
    // if no data block -> error
    // allocate new inode
    // allocate new data block

    return 0;
}

int superblock_init() {
    struct superblock superblock = {.file_system_type = FS_TYPE,
                                    .num_inodes = NUM_INODES,
                                    .inodes_table_start = INODE_TABLE_START,
                                    .num_data_blocks = NUM_DATA_BLOCKS,
                                    .block_size = BLOCK_SIZE};
    block buf = {0};
    memcpy(buf, &superblock, sizeof(struct superblock));
    block_write(&buf, 0);

    return 0;
}

int root_dir_init() {
    int errCode;
    // bootstrap root dir
    int data_block = data_block_alloc();

    if (data_block == -1) {
        // something bad happened...
        return -1;
    }

    struct extent ext = {
        .logical_start = 0, .data_start = data_block, .block_count = 1};

    struct inode root_inode = {.file_type = DIRECTORY_T,
                               .parent_inode = ROOT_INODE,  // self reference
                               .size = 0,
                               .blocks_occupied = 1,
                               .extent_count = 1,
                               .extents = {ext}};

    errCode = inode_alloc(&root_inode);
    if (errCode != 0) {
        // something bad happened....
        // maybe we should check to ensure that inode num is 0 too
        // rollback
        data_block_free(data_block);
        return -1;
    }

    struct dirent curr_dir = {
        .valid = true,
        .filename = ".",
        .inode = ROOT_INODE,
    };
    if (dir_add(ROOT_INODE, &curr_dir) != 0) {
        return -1;
    }

    struct dirent par_dir = {
        .valid = true,
        .filename = "..",
        .inode = ROOT_INODE,
    };
    if (dir_add(ROOT_INODE, &par_dir) != 0) {
        return -1;
    }

    // success
    return 0;
}
