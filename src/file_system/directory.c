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
 * returns the inode number if directory entry matching filename found, else -1.
 */
int dir_lookup(int inode_num, const char* filename) {
    if (filename == NULL) {
        fprintf(stderr, "Warning: filename cannot be NULL\n");
        return -1;
    }

    if (inode_num < 0 || inode_num >= NUM_INODES) {
        fprintf(stderr, "Warning: inode number %d is out of bounds\n",
                inode_num);
        return -1;
    }

    struct inode inode;
    if (inode_read(&inode, inode_num) != 0) {
        return -1;
    }

    if (inode.file_type != DIRECTORY_T) {
        fprintf(stderr, "Warning: the inode at slot %d is not a directory\n",
                inode_num);
        return -1;
    }

    for (int i = 0; i < inode.extent_count; i++) {
        extent ext = inode.extents[i];

        for (int b = 0; b < ext.block_count; b++) {
            block data;
            int data_block = ext.data_start + b;

            if (data_block_read(&data, data_block) != 0) {
                return -1;
            }

            for (int offset = 0; offset < BLOCK_SIZE;
                 offset += sizeof(struct dirent)) {
                struct dirent dirent;
                memcpy(&dirent, data + offset, sizeof(struct dirent));
                // ensure that directory is a valid entry
                if (dirent.valid && strcmp(dirent.filename, filename) == 0) {
                    return dirent.inode;
                }
            }
        }
    }

    return -1;
}

/*
 * adds a directory entry to the specified inode's data.
 *
 * returns 0 in case of success, else -1.
 */
int dir_add(int inum, struct dirent* new_dirent) {
    if (new_dirent == NULL) {
        fprintf(stderr, "Warning: new directory entry cannot be NULL\n");
        return -1;
    }

    if (inum < 0 || inum >= NUM_INODES) {
        fprintf(stderr, "Warning: inode number %d is out of bounds\n",
                inum);
        return -1;
    }

    struct inode inode;
    if (inode_read(&inode, inum) != 0) {
        return -1;
    }

    if (inode.file_type != DIRECTORY_T) {
        fprintf(stderr, "Warning: the inode at slot %d is not a directory\n",
                inum);
        return -1;
    }

    for (int i = 0; i < inode.extent_count; i++) {
        extent ext = inode.extents[i];

        for (int b = 0; b < ext.block_count; b++) {
            block data;
            int data_block = ext.data_start + b;

            if (data_block_read(&data, data_block) != 0) {
                return -1;
            }

            for (int offset = 0; offset < BLOCK_SIZE;
                 offset += sizeof(struct dirent)) {
                struct dirent dirent;
                memcpy(&dirent, data + offset, sizeof(struct dirent));

                // found duplicate
                if (dirent.valid &&
                    strcmp(dirent.filename, new_dirent->filename) == 0) {
                    fprintf(
                        stderr,
                        "Warning: filename %s already exists in directory\n",
                        new_dirent->filename);
                    return -1;
                }

                // found a hole
                if (!(dirent.valid)) {
                    memcpy(data + offset, new_dirent, sizeof(struct dirent));

                    if (data_block_write(&data, data_block) != 0) {
                        return -1;
                    }

                    return 0;
                }
            }
        }
    }

    // no holes found, need to allocate a new block
    int new_data_block = data_block_alloc();
    if (new_data_block == -1) {
        return -1;
    }

    // update inode extents list
    if (inode.extent_count > 0) {
        // case: inode has existing extents
        // peek last extent in extent list
        struct extent* ext = &inode.extents[inode.extent_count - 1];
        // update extent list
        if (new_data_block == ext->data_start + ext->block_count) {
            // new block can extend last extent
            ext->block_count++;
        } else if (inode.extent_count < MAX_EXTENTS) {
            // need to create a new extent
            inode.extents[inode.extent_count] =
                (struct extent){.logical_start = inode.blocks_occupied,
                                .data_start = new_data_block,
                                .block_count = 1};
            inode.extent_count++;
        } else {
            fprintf(stderr, "Warning: could not update extent list of inode %d",
                    inum);
            // rollback
            data_block_free(new_data_block);
            return -1;
        }
    } else {
        // case: no extents in inode yet
        // create new one
        struct extent ext = {
            .logical_start = 0,
            .data_start = new_data_block,
            .block_count = 1,
        };
        inode.extent_count++;
        memcpy(inode.extents, &ext, sizeof(struct extent));
    }

    inode.blocks_occupied++;

    // add new entry to block at first slot (since block newly allocated)
    block data = {0};
    memcpy(data, new_dirent, sizeof(struct dirent));
    inode.size += sizeof(struct dirent);

    if (data_block_write(&data, new_data_block) != 0) {
        // rollback
        if (data_block_free(new_data_block) != 0) {
            fprintf(stderr,
                    "Warning: failed to free newly allocated block during "
                    "rollback\n");
        }
        return -1;
    }

    if (inode_write(&inode, inum) != 0) {
        // rollback
        if (data_block_free(new_data_block) != 0) {
            fprintf(stderr,
                    "Warning: failed to free newly allocated block during "
                    "rollback\n");
        }
        return -1;
    }

    return 0;
}

/*
 * removes a directory entry from the specified inode by invalidating
 * it. does not perform any type of compaction in the directory's data
 * blocks.
 *
 * returns 0 in case of success, else -1.
 */
int dir_remove(int inode_num, const char* filename) {
    if (filename == NULL) {
        fprintf(stderr, "Warning: filename cannot be NULL\n");
        return -1;
    }

    if (inode_num < 0 || inode_num >= NUM_INODES) {
        fprintf(stderr, "Warning: inode number %d is out of bounds\n",
                inode_num);
        return -1;
    }

    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        fprintf(stderr, "Warning: cannot remove protected directory %s\n",
                filename);
        return -1;
    }

    struct inode inode;
    if (inode_read(&inode, inode_num) != 0) {
        return -1;
    }

    if (inode.file_type != DIRECTORY_T) {
        fprintf(stderr, "Warning: the inode at slot %d is not a directory\n",
                inode_num);
        return -1;
    }

    // scan all entries via inode extents to find location
    for (int i = 0; i < inode.extent_count; i++) {
        extent ext = inode.extents[i];

        for (int b = 0; b < ext.block_count; b++) {
            block data;
            int data_block = ext.data_start + b;

            if (data_block_read(&data, data_block) != 0) {
                return -1;
            }

            for (int offset = 0; offset < BLOCK_SIZE;
                 offset += sizeof(struct dirent)) {
                struct dirent dirent;
                memcpy(&dirent, data + offset, sizeof(struct dirent));

                // found entry to delete
                if (dirent.valid && strcmp(dirent.filename, filename) == 0) {
                    dirent.valid = false;
                    // make snapshot copy of block
                    block before_snapshot;
                    memcpy(before_snapshot, data, sizeof(block));

                    // write invalidated entry
                    memcpy(data + offset, &dirent, sizeof(struct dirent));

                    if (data_block_write(&data, data_block) != 0) {
                        return -1;
                    }

                    // data write was success, update metadata
                    inode.size -= sizeof(struct dirent);

                    if (inode_write(&inode, inode_num) != 0) {
                        // rollback
                        if (data_block_write(&before_snapshot, data_block) !=
                            0) {
                            fprintf(stderr,
                                    "Warning: failed to rollback to previous "
                                    "snapshot at block %d due to inode write "
                                    "failure\n",
                                    data_block);
                        }
                        return -1;
                    }

                    return 0;
                }
            }
        }
    }

    // did not find entry, it does not exist
    fprintf(stderr, "Warning: filename %s does not exist in directory\n",
            filename);
    return -1;

    // TODO: for now, we leave the deleted entry as a hole. in the future, maybe
    // we will implement compaction to optimize the space used and free up some
    // data blocks if we can.
}
