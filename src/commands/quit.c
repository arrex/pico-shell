#include <stdio.h>
#include <stdlib.h>

#include "../memory/memory.h"

int quit() {
    printf("Bye!\n");

    mem_deinit();

    exit(0);
}
