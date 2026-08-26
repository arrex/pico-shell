#include "file_system.h"

typedef struct dir_entry {
    char filename[MAX_FILENAME_LEN];
    int inode;
} dir_entry;

// forward decls
struct inode;

// TODO: implement
int dir_lookup(char* filename);
int dir_add(int inode, struct dir_entry* new_entry);
// TODO: implement
int dir_remove(char* filename);
