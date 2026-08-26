#include "file_system.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// our own way of emulating the disk
uint8_t disk[NUM_BLOCKS * BLOCK_SIZE] = {0};

/*
 * performs a read operation on our disk abstraction.
 *
 * returns 0 in case of success, else -1.
 */
int disk_read(void* buf, int size, int offset) {
    if (buf == NULL) {
        fprintf(stderr, "Warning: input buffer is null\n");
        return -1;
    }

    if (size > sizeof(disk) - offset) {
        fprintf(stderr, "Warning: not enough space on disk to write %d bytes with %d offset\n", size, offset);
        return -1;
    }

    if (offset > sizeof(disk)) {
        fprintf(stderr, "Warning: offset %d exceeds size of disk\n", offset);
        return -1;
    }

    memcpy(buf, &disk[offset], size);
    return 0;
}

int disk_write(const void* buf, int size, int offset) {
    if (buf == NULL) {
        fprintf(stderr, "Warning: input buffer is null\n");
        return -1;
    }

    if (size > sizeof(disk) - offset) {
        fprintf(stderr, "Warning: not enough space on disk to write %d bytes with %d offset\n", size, offset);
        return -1;
    }

    if (offset > sizeof(disk)) {
        fprintf(stderr, "Warning: offset %d exceeds size of disk\n", offset);
        return -1;
    }

    memcpy(&disk[offset], buf, size);
    return 0;
}
