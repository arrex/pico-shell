#include "disk.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// our own way of emulating the disk
uint8_t disk[DISK_SIZE] = {0};

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

    if (size < 0 || size > DISK_SIZE - offset) {
        fprintf(stderr, "Warning: invalid size %d\n", size);
        return -1;
    }

    if (offset < 0 || offset >= DISK_SIZE) {
        fprintf(stderr, "Warning: invalid offset %d\n", offset);
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

    if (size < 0 || size > DISK_SIZE - offset) {
        fprintf(stderr, "Warning: invalid size %d\n", size);
        return -1;
    }

    if (offset < 0 || offset >= DISK_SIZE) {
        fprintf(stderr, "Warning: invalid offset %d\n", offset);
        return -1;
    }
    memcpy(&disk[offset], buf, size);
    return 0;
}

#ifdef PICOSHELL_TESTING
void disk_reset(void) {
    memset(disk, 0, DISK_SIZE);
}
#endif
