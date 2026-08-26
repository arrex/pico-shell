#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../scheduling/pcb.h"
#include "frame_store.h"
#include "lru.h"
#include "memory.h"
#include "variable_store.h"

struct memory_manager* mem;

void mem_init() {
    mem = malloc(sizeof(struct memory_manager));

    // init deps
    struct variable_store* vstore = vstore_init();
    struct frame_store* fstore = fstore_init();

    // inject deps
    mem->vstore = vstore;
    mem->fstore = fstore;
}

void mem_deinit() {
    vstore_deinit(mem->vstore);
    fstore_deinit(mem->fstore);

    mem->vstore = NULL;
    mem->fstore = NULL;

    free(mem);
}

char* mem_get_fstore_value(int address) {
    struct frame_store* fstore = mem->fstore;
    return fstore_get(fstore, address);
}

void mem_set_fstore_value(char* value_in, int address) {
    struct frame_store* fstore = mem->fstore;
    fstore_set(fstore, value_in, address);
}

char* mem_get_vstore_value(char* var_in) {
    struct variable_store* vstore = mem->vstore;
    return vstore_get(vstore, var_in);
}

void mem_set_vstore_value(char* var_in, char* value_in) {
    struct variable_store* vstore = mem->vstore;
    vstore_set(vstore, var_in, value_in);
}

void mem_load_page_into_frame(struct PCB* pcb, int page, int frame) {
    if (page < 0 || page * FRAME_SIZE >= pcb->file_length) {
        printf("Warning: the page %d is not within range of file %s\n", page,
               pcb->filename);
        return;
    }

    for (int offset = 0; offset < FRAME_SIZE; offset++) {
        int line = page * FRAME_SIZE + offset;
        int address = frame * FRAME_SIZE + offset;

        if (line < pcb->file_length) {
            mem_set_fstore_value(pcb->file_contents[line], address);
        }
    }

    // upate frame table metadata
    mem->fstore->frame_table[frame].page = page;
    mem->fstore->frame_table[frame].pcb = pcb;

    // update pcb page table
    pcb->page_table[page] = frame;

    // update LRU ordering
    access_frame(frame);
}

int mem_find_available_frame() {
    struct frame_store* fstore = mem->fstore;
    return fstore_find_available_frame(fstore);
}

void mem_free_memory_frame(int frame) {
    struct frame_store* fstore = mem->fstore;
    fstore_evict_frame(fstore, frame);

    // invalidate PCB page table entry
    struct PCB* pcb = fstore->frame_table[frame].pcb;
    int page = fstore->frame_table[frame].page;
    pcb->page_table[page] = -1;

    // invalidate frame table metadata to prevent staleness
    fstore->frame_table[frame].page = -1;
    fstore->frame_table[frame].pcb = NULL;
}
