#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "block.h"
#include "data.h"
#include "directory.h"
#include "inode.h"
#include "pathname.h"

int superblock_init();
int root_init();

struct inode cwd;

/*
 * initializes the entire file system by writing the superblock and the
 * root directory entry.
 *
 * returns 0 in case of success, else -1.
 */
int fs_init() {
    if (superblock_init() != 0) {
        fprintf(stderr, "[fs] error: failed to init superblock\n");
        return -1;
    }

    if (root_init() != 0) {
        fprintf(stderr, "[fs] error: failed to init root directory\n");
        return -1;
    }

    // set cwd to root
    if (inode_read(&cwd, ROOT_INODE) != 0) {
        fprintf(stderr, "[fs] error: failed to set cwd to root\n");
        return -1;
    }

    return 0;
}

int superblock_init() {
    struct superblock superblock = {.fs_type = FS_TYPE,
                                    .num_inodes = NUM_INODES,
                                    .inodes_table_start = INODE_TABLE_START,
                                    .num_data_blocks = NUM_DATA_BLOCKS,
                                    .block_size = BLOCK_SIZE};

    block buf;
    memcpy(buf, &superblock, sizeof(struct superblock));

    if (block_write(&buf, 0) != 0) {
        return -1;
    }

    return 0;
}

/*
 * initializes the root inode. we don't want to use the file system's create
 * primitive for this dir since it is a special case. we don't want to allow
 * users to create "/".
 *
 * TODO: find a way to streamline this
 */
int root_init() {
    struct inode root = {
        .type = DIRECTORY_T,
    };
    // invalidate all direct ptrs
    for (int i = 0; i < NDIRECT; i++) {
        root.addrs[i] = -1;
    }

    if (inode_alloc(&root) != 0) {
        return -1;
    }

    if (root.inum != ROOT_INODE) {
        fprintf(
            stderr,
            "[fs] error: root inode was allocated to actual %d, expected %d\n",
            root.inum, ROOT_INODE);
        return -1;
    }

    // add dot entries
    struct dirent curr = {
        .valid = true,
        .name = ".",
        .inum = root.inum,
    };

    struct dirent par = {
        .valid = true,
        .name = "..",
        .inum = root.inum,
    };

    if (dir_add(root.inum, &curr) != 0 || dir_add(root.inum, &par) != 0) {
        return -1;
    }

    return 0;
}

int fs_chdir(char* path) {
    if (path == NULL) {
        fprintf(stderr, "[fs] error: path cannot be null\n");
        return -1;
    }

    struct inode dest_inode;
    path_lookup(&dest_inode, path);
    if (dest_inode.type != DIRECTORY_T) {
        fprintf(stderr, "[fs] error: destination is not a directory\n");
        return -1;
    }

    // change cwd to destination inode
    cwd = dest_inode;
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
        fprintf(stderr, "[fs] error: path cannot be null\n");
        return -1;
    }

    if (type != FILE_T && type != DIRECTORY_T) {
        // %d specifier since enum types treated as int
        fprintf(stderr, "[fs] error: invalid file type %d\n", type);
        return -1;
    }

    struct inode inode = {
        .type = type,
        .num_links =
            1,  // init to 1 since parent has dirent that points to child
    };
    // invalidate all direct ptrs
    for (int i = 0; i < NDIRECT; i++) {
        inode.addrs[i] = -1;
    }

    struct inode dir_inode;
    char name[MAX_FILENAME_LEN];

    if ((path_lookup_parent(&dir_inode, path, name)) != 0) {
        return -1;
    }

    if (dir_lookup(dir_inode.inum, name) != -1) {
        fprintf(stderr,
                "[fs] error: entry with name %s already exists in directory\n",
                name);
        return -1;
    }

    if (inode_alloc(&inode) == -1) {
        return -1;
    }

    // add dot entries if creating a dir
    if (type == DIRECTORY_T) {
        struct dirent curr = {
            .valid = true,
            .name = ".",
            .inum = inode.inum,
        };

        struct dirent par = {
            .valid = true,
            .name = "..",
            .inum = dir_inode.inum,
        };

        if (dir_add(inode.inum, &curr) != 0 || dir_add(inode.inum, &par) != 0) {
            return -1;
        }

        // update parent's num links since ".." dirent in child points to parent
        dir_inode.num_links++;
        if (inode_update(&dir_inode, dir_inode.inum) != 0) {
            return -1;
        }

        // link new entry in parent dir
        struct dirent par_link = {
            .valid = true,
            .inum = inode.inum,
        };
        memcpy(&par_link.name, name, MAX_FILENAME_LEN);
        if (dir_add(dir_inode.inum, &par_link) != 0) {
            return -1;
        }
    }

    return 0;
}

/*
 * deletes a resource at the specified path. resource could be a file or a
 * directory.
 *
 * returns 0 in case of success, else -1.
 */
int fs_delete(char* path, enum file_type type) {
    if (path == NULL) {
        fprintf(stderr, "[fs] error: path cannot be null\n");
        return -1;
    }

    if (strcmp(path, "/") == 0) {
        fprintf(stderr, "[fs] error: cannot delete root directory\n");
        return -1;
    }

    if (type != FILE_T && type != DIRECTORY_T) {
        fprintf(stderr, "[fs] error: invalid file type %d\n", type);
        return -1;
    }

    struct inode dir_inode, inode;
    char name[MAX_FILENAME_LEN];

    if (path_lookup_parent(&dir_inode, path, name) != 0) {
        return -1;
    }

    int inum = dir_lookup(dir_inode.inum, name);
    if (inum == -1) {
        fprintf(stderr,
                "[fs] error: entry with name %s does not exist in directory\n",
                name);
        return -1;
    }

    if (inode_read(&inode, inum) != 0) {
        return -1;
    }

    // if component is dir, check if it is empty first
    if (inode.type == DIRECTORY_T && dir_empty(inode.inum) != 1) {
        fprintf(
            stderr,
            "[fs] error: directory is not empty, remove its contents first\n");
        return -1;
    }

    // delete dirent from parent dir
    if (dir_remove(dir_inode.inum, name) != 0) {
        return -1;
    }

    // if child is dir, then it had ".." entry pointing to parent
    if (inode.type == DIRECTORY_T) {
        dir_inode.num_links--;
        if (inode_update(&dir_inode, dir_inode.inum) != 0) {
            return -1;
        }
    }

    // free all data data blocks held by resource
    for (int i = 0; i < NDIRECT; i++) {
        if (data_free(inode.addrs[i]) != 0) {
            return -1;
        }
    }

    // free resource inode
    if (inode_free(inode.inum) == -1) {
        return -1;
    }

    return 0;
}
