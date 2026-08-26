#include "frame_store.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lru.h"
#include "memory.h"

struct frame_store* fstore_init() {
    // init lru strategy
    lru_init();

    struct frame_store* frame_store = malloc(sizeof(struct frame_store));

    for (int i = 0; i < FRAME_STORE_SIZE; i++) {
        frame_store->memory[i] = NULL;
    }

    for (int i = 0; i < FRAME_STORE_SIZE / FRAME_SIZE; i++) {
        frame_store->frame_table[i].page = -1;
        frame_store->frame_table[i].pcb = NULL;
    }

    return frame_store;
}

void fstore_deinit(struct frame_store* fstore) {
    // deinit in reverse order of init
    free(fstore);
    lru_deinit();
}

/*
 * This function searches for a free frame in shell memory.
 * Returns frame number if free frame is found, -1 otherwise.
 */
int fstore_find_available_frame(struct frame_store* fstore) {
    for (int frame = 0; frame < FRAME_STORE_SIZE / FRAME_SIZE; frame++) {
        int address = frame * FRAME_SIZE;

        if (fstore->memory[address] == NULL) {
            return frame;
        }
    }

    // No free frame found
    return -1;
}

/*
 * This function returns the string stored at a particular address. Returns
 * NULL if address is not used.
 */
char* fstore_get(struct frame_store* fstore, int address) {
    char* value = fstore->memory[address];

    if (value != NULL) {
        return strdup(value);
    }

    return NULL;
}

/*
 * This function inserts a new entry into shell memory.
 */
void fstore_set(struct frame_store* fstore, char* value_in, int address) {
    if (address < 0 || address > FRAME_STORE_SIZE) {
        printf("Warning: address %d does not fall within frame store bounds\n",
               address);
        return;
    }

    fstore->memory[address] = strdup(value_in);
}

void fstore_evict_frame(struct frame_store* fstore, int frame) {
    int base = frame * FRAME_SIZE;

    for (int offset = 0; offset < FRAME_SIZE; offset++) {
        int address = base + offset;
        free(fstore->memory[address]);
        fstore->memory[address] = NULL;
    }
}
