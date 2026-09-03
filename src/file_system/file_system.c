#include "file_system.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block_layer.h"
#include "pathname.h"
#include "data_block.h"
#include "directory.h"
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
                               .inum = ROOT_INODE,
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

/*
 * creates a file of the specified type. we expose a single create primitive in
 * our file system since directories are essentially structured files.
 *
 * returns 0 in case of success, else -1.
 */
int fs_create(char* path, enum file_type type) {
    if (path == NULL) {
        fprintf(stderr, "Warning: path cannot be null\n");
        return -1;
    }

    if (type != FILE_T && type != DIRECTORY_T) {
        // %d specifier since enum types treated as int
        fprintf(stderr, "Warning: invalid file type %d\n", type);
        return -1;
    }

    struct inode inode = {
        .file_type = type,
        .inum = -1, // will be mutated by inode_alloc
        .size = 0,
        .blocks_occupied = 0,
        .extent_count = 0,
        .extents = {},
    };
    struct inode dir_inode;
    char name[MAX_FILENAME_LEN];

    if ((path_lookup_parent(&dir_inode, path, name)) != 0) {
        return -1;
    }

    if (dir_lookup(dir_inode.inum, name) != -1) {
        fprintf(stderr, "Warning: entry with name %s already exists in directory\n", name);
        return -1;
    }

    if (inode_alloc(&inode) == -1) {
        return -1;
    }

    // add dot entries if creating a dir
    if (type == DIRECTORY_T) {
        struct dirent curr_dir = {
            .valid = true,
            .filename = ".",
            .inode = inode.inum,
        };

        if (dir_add(inode.inum, &curr_dir) != 0) {
            return -1;
        }

        // link to parent dir
        struct dirent par_dir = {
            .valid = true,
            .filename = "..",
            .inode = dir_inode.inum,
        };

        if (dir_add(inode.inum, &par_dir) != 0) {
            return -1;
        }

        // link new entry in parent dir
        struct dirent par_link = {
            .valid = true,
            .inode = inode.inum,
        };
        memcpy(&par_link.filename, name, MAX_FILENAME_LEN);

        if (dir_add(dir_inode.inum, &par_link) != 0)  {
            return -1;
        }
    }

    return 0;
}
