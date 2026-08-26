#include "memory.h"

// computes the number of frames in memory based on total memory lines and size
// of a frame
#define FRAME_TABLE_SIZE FRAME_STORE_SIZE / FRAME_SIZE

typedef struct frame_table_entry {
    int page;
    struct PCB* pcb;
} frame_table_entry;

typedef struct frame_store {
    // Frame store memory will store string entries
    char* memory[FRAME_STORE_SIZE];
    // Table tracking shell memory frame metadata
    struct frame_table_entry frame_table[FRAME_TABLE_SIZE];
} frame_store;

struct frame_store* fstore_init();
void fstore_deinit(struct frame_store* fstore);
int fstore_find_available_frame(struct frame_store* fstore);
char* fstore_get(struct frame_store* fstore, int address);
void fstore_set(struct frame_store* fstore, char* value_in, int address);
void fstore_evict_frame(struct frame_store* fstore, int frame);
