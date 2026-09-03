#include "inode.h"

int path_lookup(struct inode* out, char* path);
int path_lookup_parent(struct inode* out, char* path, char* name);
