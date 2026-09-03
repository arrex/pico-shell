#include <stdbool.h>

#include "file_system.h"

typedef struct dirent {
    bool valid;
    char filename[MAX_FILENAME_LEN];
    int inode;
} dirent;

// forward decls
struct inode;

int dir_lookup(int inode_num, const char* filename);
int dir_add(int inode_num, struct dirent* new_dirent);
int dir_remove(int inode_num, const char* filename);
