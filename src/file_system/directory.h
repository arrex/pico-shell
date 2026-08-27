#include <stdbool.h>

#include "file_system.h"

// our file system design
typedef struct dirent {
    char filename[MAX_FILENAME_LEN];
    int inode;
} dirent;

// forward decls
struct inode;

bool dir_lookup(int inode_num, char* filename);
int dir_add(int inode_num, struct dirent* new_dirent);
int dir_remove(int inode_num, char* filename);
