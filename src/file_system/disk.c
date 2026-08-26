#include "fs_specs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// our own way of emulating the disk
uint8_t disk[NUM_BLOCKS * BLOCK_SIZE] = {0};

void disk_read(void* buf, int size, int offset) {
    memcpy(buf, &disk[offset], size);
}

void disk_write(const void* buf, int size, int offset) {
    memcpy(&disk[offset], buf, size);
}
