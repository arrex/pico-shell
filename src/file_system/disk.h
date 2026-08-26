#pragma once

#include "file_system.h"

#define DISK_SIZE (NUM_BLOCKS * BLOCK_SIZE)

int disk_read(void* buf, int size, int offset);
int disk_write(const void* buf, int size, int offset);

// functions exposed for unit testing orchestration
#ifdef PICOSHELL_TESTING
void disk_reset(void);
#endif
