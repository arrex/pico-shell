#include "block_layer.h"

int data_block_alloc();
int data_block_free(int data_block);
int data_block_read(block* out, int data_block);
int data_block_write(block* in, int data_block);
