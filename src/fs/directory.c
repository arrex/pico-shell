#include "directory.h"

#include <stdio.h>
#include <string.h>

#include "../utils/utils.h"
#include "block.h"
#include "data.h"
#include "file.h"
#include "inode.h"

// TODO: add rollback mechanism for fault tolerance, impl journaling later

/*
 * scans all the directory entries of the specified inode and matches on
 * given filename.
 *
 * returns the inode number if directory entry matching filename found, else -1.
 */
int dir_lookup(uint inum, const char* name) {
    if (name == NULL) {
        fprintf(stderr, "[directory] error: filename cannot be NULL\n");
        return -1;
    }

    if (inum >= NUM_INODES) {
        fprintf(stderr, "[directory] error: inode number %d is out of bounds\n",
                inum);
        return -1;
    }

    struct inode inode;
    if (inode_read(&inode, inum) != 0) {
        return -1;
    }

    if (inode.type != DIRECTORY_T) {
        fprintf(stderr,
                "[directory] error: the inode at slot %d is not a directory\n",
                inum);
        return -1;
    }

    struct dirent dirent;
    for (int offset = 0; offset < inode.size; offset += sizeof(dirent)) {
        if (file_read(&inode, (char*)&dirent, offset, sizeof(dirent)) !=
            sizeof(dirent)) {
            return -1;
        }

        if (dirent.valid && strcmp(dirent.name, name) == 0) {
            return dirent.inum;
        }
    }

    return -1;
}

/*
 * scans the contents of a directory given the inode number to determine if it
 * is empty or not.
 *
 * returns 1 if the directory is empty, 0 if it isn't, and -1 in case of error.
 */
int dir_empty(uint inum) {
    if (inum >= NUM_INODES) {
        fprintf(stderr, "[directory] error: inode number %d is out of bounds\n",
                inum);
        return -1;
    }

    struct inode inode;
    if (inode_read(&inode, inum) != 0) {
        return -1;
    }

    if (inode.type != DIRECTORY_T) {
        fprintf(stderr,
                "[directory] error: the inode at slot %d is not a directory\n",
                inum);
        return -1;
    }

    struct dirent dirent;
    for (int offset = 0; offset < inode.size; offset += sizeof(dirent)) {
        if (file_read(&inode, (char*)&dirent, offset, sizeof(dirent)) !=
            sizeof(dirent)) {
            return -1;
        }

        if (dirent.valid && strcmp(dirent.name, ".") != 0 &&
            strcmp(dirent.name, "..") != 0) {
            return 0;
        }
    }

    return 1;
}

/*
 * adds a directory entry to the specified directory inode's data region.
 *
 * returns 0 in case of success, else -1.
 */
int dir_add(uint inum, const struct dirent* new_dirent) {
    if (new_dirent == NULL) {
        fprintf(stderr,
                "[directory] error: new directory entry cannot be NULL\n");
        return -1;
    }

    if (inum >= NUM_INODES) {
        fprintf(stderr, "[directory] error: inode number %d is out of bounds\n",
                inum);
        return -1;
    }

    struct inode inode;
    if (inode_read(&inode, inum) != 0) {
        return -1;
    }

    if (inode.type != DIRECTORY_T) {
        fprintf(stderr,
                "[directory] error: the inode at slot %d is not a directory\n",
                inum);
        return -1;
    }

    // found duplicate
    if (dir_lookup(inum, new_dirent->name) != -1) {
        return -1;
    }

    uint offset;
    struct dirent dirent;
    for (offset = 0; offset < inode.size; offset += sizeof(dirent)) {
        if (file_read(&inode, (char*)&dirent, offset, sizeof(dirent)) !=
            sizeof(dirent)) {
            return -1;
        }

        // found hole
        if (!dirent.valid) {
            if (file_write(&inode, (char*)&new_dirent, offset,
                           sizeof(dirent)) != sizeof(dirent)) {
                return -1;
            }

            return 0;
        }
    }

    // did not find any holes in inode's allocated space
    // write at offset, this file layer will alloc space
    if (file_write(&inode, (char*)&new_dirent, offset, sizeof(dirent)) !=
        sizeof(dirent)) {
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
int dir_remove(uint inum, const char* name) {
    if (name == NULL) {
        fprintf(stderr, "[directory] error: filename cannot be NULL\n");
        return -1;
    }

    if (inum >= NUM_INODES) {
        fprintf(stderr, "[directory] error: inode number %d is out of bounds\n",
                inum);
        return -1;
    }

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        fprintf(stderr,
                "[directory] error: cannot remove protected directory %s\n",
                name);
        return -1;
    }

    struct inode inode;
    if (inode_read(&inode, inum) != 0) {
        return -1;
    }

    if (inode.type != DIRECTORY_T) {
        fprintf(stderr,
                "[directory] error: the inode at slot %d is not a directory\n",
                inum);
        return -1;
    }

    struct dirent dirent;
    for (int offset = 0; offset < inode.size; offset += sizeof(dirent)) {
        if (file_read(&inode, (char*)&dirent, offset, sizeof(dirent)) !=
            sizeof(dirent)) {
            return -1;
        }

        // found match
        if (strcmp(dirent.name, name) == 0) {
            memset(&dirent, 0, sizeof(dirent));

            if (file_write(&inode, (char*)&dirent, offset, sizeof(dirent)) !=
                sizeof(dirent)) {
                return -1;
            }

            return 0;
        }
    }

    // did not find entry, it does not exist
    fprintf(stderr,
            "[directory] error: filename %s does not exist in directory\n",
            name);
    return -1;

    // TODO: for now, we leave the deleted entry as a hole. in the future, maybe
    // we will implement compaction to optimize the space used and free up some
    // data blocks if we can.
}
