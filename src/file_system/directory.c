#include "directory.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "../utils/utils.h"
#include "block_layer.h"
#include "data_block.h"
#include "file_system.h"
#include "inode.h"

/*
 * scans all the directory entries of the specified inode and matches on
 * given filename.
 *
 * returns true if directory entry matching filename found, else false.
 *
 * ensure that specified inode is of type directory.
 */
bool dir_lookup(int inode_num, char* filename) {
    if (filename == NULL) {
        fprintf(stderr, "Warning: filename cannot be NULL\n");
        return false;
    }

    if (inode_num < 0 || inode_num >= NUM_INODES) {
        fprintf(stderr, "Warning: inode number %d is out of bounds\n",
                inode_num);
        return false;
    }

    struct inode inode;

    if (inode_read(&inode, inode_num) != 0) {
        return false;
    }

    if (inode.file_type != DIRECTORY_T) {
        fprintf(stderr, "Warning: the inode at slot %d is not a directory\n",
                inode_num);
        return false;
    }

    int num_entries = inode.size / sizeof(struct dirent);
    for (int i = 0; i < inode.extent_count; i++) {
        extent ext = inode.extents[i];

        for (int b = 0; b < ext.block_count; b++) {
            block data;
            int data_block = ext.data_start + b;

            if (data_block_read(&data, data_block) != 0) {
                return false;
            }

            for (int offset = 0; offset < BLOCK_SIZE;
                 offset += sizeof(struct dirent)) {
                // no entries left to process
                if (num_entries == 0) {
                    return false;
                }

                struct dirent dirent;
                memcpy(&dirent, data + offset, sizeof(struct dirent));

                if (strcmp(dirent.filename, filename) == 0) {
                    return true;
                }

                num_entries--;
            }
        }
    }

    return false;
}

int dir_add(int inode_num, struct dirent* new_dirent) {
    struct inode inode;
    if (inode_read(&inode, inode_num) != 0) {
        return -1;
    }

    if (inode.file_type != DIRECTORY_T) {
        fprintf(stderr, "Error: the inode at slot %d is not a directory\n",
                inode_num);
        return -1;
    }

    int blocks_needed = ceili(inode.size + sizeof(struct dirent), BLOCK_SIZE);
    // need to allocate a new block
    if (inode.blocks_occupied < blocks_needed) {
        int data_block = data_block_alloc();
        if (data_block == -1) {
            return -1;
        }

        int phys_block = DATA_REGION_START + data_block;

        // update extent list
        struct extent* last = &inode.extents[inode.extent_count - 1];
        if (phys_block == last->data_start + last->block_count) {
            // new block can extend last extent
            last->block_count++;
        } else if (inode.extent_count < MAX_EXTENTS) {
            // need to create a new extent
            inode.extents[inode.extent_count] =
                (struct extent){.logical_start = inode.blocks_occupied,
                                .data_start = phys_block,
                                .block_count = 1};

            inode.extent_count++;
        } else {
            fprintf(stderr, "Error: could not update extent list of inode %d",
                    inode_num);
            // rollback
            data_block_free(data_block);
            return -1;
        }

        inode.blocks_occupied++;
    }

    // add dir entry into data block
    struct extent extent = inode.extents[inode.extent_count - 1];
    int phys_block = extent.data_start + extent.block_count - 1;

    block data;
    if (block_read(&data, phys_block) != 0) {
        return -1;
    }

    // add new entry to block
    int offset = inode.size % BLOCK_SIZE;
    memcpy(&data[offset], new_dirent, sizeof(struct dirent));

    // update metadata
    inode.size += sizeof(struct dirent);

    if (block_write(&data, phys_block) != 0) {
        return -1;
    }

    if (inode_write(&inode, inode_num) != 0) {
        // TODO: rollback
        return -1;
    }

    return 0;
}
