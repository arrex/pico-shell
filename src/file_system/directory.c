#include "directory.h"

#include <stdio.h>
#include <string.h>

#include "../utils/utils.h"
#include "block_layer.h"
#include "data_block.h"
#include "file_system.h"
#include "inode.h"

int dir_add(int inode_num, struct dir_entry* new_entry) {
    struct inode inode;
    inode_read(&inode, inode_num);

    if (inode.file_type != DIRECTORY_T) {
        fprintf(stderr, "Error: the inode %d is not a directory\n", inode_num);
        return -1;
    }

    int blocks_needed =
        ceili(inode.size + sizeof(struct dir_entry), BLOCK_SIZE);
    // need to allocate a new block
    if (inode.blocks_occupied < blocks_needed) {
        int data_block = data_block_alloc();
        if (data_block == -1) {
            return -1;
        }

        int phys_block = DATA_REGION_START + data_block;

        // update extent list
        struct extent* last = &inode.extents[inode.extent_count - 1];
        if (phys_block == last->physical_start + last->block_count) {
            // new block can extend last extent
            last->block_count++;
        } else if (inode.extent_count < MAX_EXTENTS) {
            // need to create a new extent
            inode.extents[inode.extent_count] =
                (struct extent){.logical_start = inode.blocks_occupied,
                                .physical_start = phys_block,
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
    int phys_block = extent.physical_start + extent.block_count - 1;

    block data;
    block_read(&data, phys_block);

    // add new entry to block
    int offset = inode.size % BLOCK_SIZE;
    memcpy(&data[offset], new_entry, sizeof(struct dir_entry));

    // update metadata
    inode.size += sizeof(struct dir_entry);

    block_write(&data, phys_block);
    inode_write(&inode, inode_num);

    return 0;
}
