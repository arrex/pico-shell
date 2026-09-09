#include "block.h"

#define DBLOCK(dnum) \
    (dnum +          \
     DATA_REGION_START)  // calculates which physical block data block maps to

int data_alloc();
int data_free(int dnum);
int data_read(block* data, int dnum);
int data_update(block* data, int dnum);
