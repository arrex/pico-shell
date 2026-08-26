#include "variable_store.h"

#include <stdlib.h>
#include <string.h>

#include "memory.h"

struct variable_store* vstore_init() {
    struct variable_store* var_store = malloc(sizeof(struct variable_store));

    for (int i = 0; i < VAR_STORE_SIZE; i++) {
        var_store->var_table[i].var = NULL;
        var_store->var_table[i].value = NULL;
    }

    return var_store;
}

void vstore_deinit(struct variable_store* vstore) { free(vstore); }

/*
 * Get value of a variable from the store based on input key
 */
char* vstore_get(struct variable_store* var_store, char* key) {
    for (int i = 0; i < VAR_STORE_SIZE; i++) {
        struct var_table_entry* entry = &var_store->var_table[i];

        if (entry->var != NULL && strcmp(entry->var, key) == 0) {
            return strdup(entry->value);
        }
    }

    // Case variable not found
    return NULL;
}

/*
 * This function either creates a new entry in the variable table
 * or updates the current variable assignment. It also updates shell
 *  memory accordingly.
 */
void vstore_set(struct variable_store* var_store, char* var_in,
                char* value_in) {
    // Linear search variable table for given variable name
    for (int i = 0; i < VAR_STORE_SIZE; i++) {
        struct var_table_entry* entry = &var_store->var_table[i];

        if (entry->var != NULL && strcmp(entry->var, var_in) == 0) {
            free(entry->value);
            entry->value = strdup(value_in);
            return;
        }
    }

    // Variable does not exist, need to find a free spot
    for (int i = 0; i < VAR_STORE_SIZE; i++) {
        struct var_table_entry* entry = &var_store->var_table[i];

        if (entry->var == NULL) {
            entry->var = strdup(var_in);
            entry->value = strdup(value_in);
            return;
        }
    }

    // Did not manage to insert into memory
    return;
}
