#include <stdbool.h>

#include "../types/types.h"
#include "fs.h"

typedef struct dirent {
    bool valid;
    char name[MAX_FILENAME_LEN];
    int inum;
} dirent;

int dir_lookup(uint inum, const char* name);
int dir_empty(uint inum);
int dir_add(uint inum, const struct dirent* dirent);
int dir_remove(uint inum, const char* name);
