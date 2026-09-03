#include <stdio.h>

#include "../utils/badcommand.h"
#include "../utils/utils.h"

int touch(char* filename) {
    if (is_valid_path(filename) != 1) {
        return badcommandNotAlphanum();
    }

    FILE* fptr;
    fptr = fopen(filename, "w");

    // Error creating file
    if (fptr == NULL) {
        return badCommandErrorOccurred();
    }

    // Error closing file
    if (fclose(fptr) != 0) {
        return badCommandErrorOccurred();
    }

    return 0;
}
