#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "directory.h"
#include "file_system.h"
#include "inode.h"

int resolve(struct inode* out, char* path, bool parent, char* name);
static char* consume(char* path, char* name);

/*
 * resolves the path to return the inode of the most nested component.
 *
 * returns 0 in case of success, else -1.
 */
int path_lookup(struct inode* out, char* path) {
    char name[MAX_FILENAME_LEN];
    return resolve(out, path, false, name);
}

/*
 * resolves the path to return the parent inode of the most nested component.
 *
 * returns 0 in case of success, else -1.
 */
int path_lookup_parent(struct inode* out, char* path, char* name) {
    return resolve(out, path, true, name);
}

/*
 * resolves the input path to find the inode of the desired component. if
 * the parent bool flag is true, will fetch the parent dir's inode (i.e.
 * 2nd to last component in the path). else, it will fetch the most nested
 * component's inode.
 *
 * returns 0 in case of success, else -1.
 */
int resolve(struct inode* out, char* path, bool parent, char* name) {
    struct inode curr, next;

    if (*path == '/') {
        // case: absolute path given
        if (inode_read(&curr, ROOT_INODE) != 0) {
            return -1;
        }
    } else {
        // TODO: implement relative path handling
        fprintf(stderr, "Warning: relative path handling not implemented\n");
        return -1;
    }

    while ((path = consume(path, name)) != NULL) {
        // not a dir
        if (curr.file_type != DIRECTORY_T) {
            return -1;
        }

        // if requesting parent dir, stop one level early
        if (parent && *path == '\0') {
            memcpy(out, &curr, sizeof(struct inode));
            return 0;
        }

        // fetch consumed path component's inode
        int next_inum = dir_lookup(curr.inum, name);
        if (next_inum == -1) {
            fprintf(stderr, "chigga we did not find the dirent\n");
            return -1;
        }

        if (inode_read(&next, next_inum) != 0) {
            return -1;
        }

        curr = next;
    }

    memcpy(out, &curr, sizeof(struct inode));
    return 0;
}

/*
 * consumes the leading component in the path and stores it in the name
 * char ptr. truncates leading slashes.
 *
 * e.g.:
 *  consume("a/bb/c", name) = "bb/c", setting name = "a"
 *  consume("///a//bb", name) = "bb", setting name = "a"
 *  consume("a", name) = "", setting name = "a"
 *  consume("", name) = consume("////", name) = 0
 *
 * returns a ptr to the next component in the path.
 */
static char* consume(char* path, char* name) {
    char* s;
    int len;

    // skip leading '/' chars
    while (*path == '/') {
        path++;
    }

    // peek if there is anything to consume
    if (*path == '\0') {
        return NULL;
    }

    // find boundaries of next path component
    s = path;
    while (*path != '/' && *path != '\0') {
        path++;
    }
    len = path - s;

    // move component into name ptr
    memmove(name, s, len);
    name[len] = '\0';  // null terminate name

    // skip trailing '/' chars
    while (*path == '/') {
        path++;
    }

    // return consumed path
    return path;
}
