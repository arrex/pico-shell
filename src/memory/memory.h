#pragma once

// Compile flag fallbacks
#ifndef FRAME_SIZE
// We use a frame size of 3 rather than a power of 2 to test interesting cases
// without having to implement more scheduling policies
#define FRAME_SIZE 3
#endif

#ifndef FRAME_STORE_SIZE
#define FRAME_STORE_SIZE 300
#endif

#ifndef VAR_STORE_SIZE
#define VAR_STORE_SIZE 10
#endif

// forward declaration -- signal existene of pcb struct type since we use it in
// shell_memory.c
struct PCB;

typedef struct memory_manager {
    struct variable_store* vstore;
    struct frame_store* fstore;
} memory_manager;

void mem_init();
void mem_deinit();
char* mem_get_fstore_value(int address);
void mem_set_fstore_value(char* value_in, int address);
char* mem_get_vstore_value(char* var_in);
void mem_set_vstore_value(char* var_in, char* value_in);
void mem_load_page_into_frame(struct PCB* pcb, int page, int frame);
int mem_find_available_frame();
void mem_free_memory_frame(int frame);
