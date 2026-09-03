#include "mkdir.h"

#include <stdlib.h>
#include <sys/stat.h>

#include "../memory/memory.h"
#include "../utils/badcommand.h"
#include "../utils/utils.h"
#include "../file_system/file_system.h"

const int mmkdir(char* path) {
    // Fetch from memory if preceded by '$' sign
    if (path[0] == '$') {
        path++;
        path = mem_get_vstore_value(path);

        if (path == NULL) {
            return badcommandVariableDoesNotExist();
        }
    }

    if (is_valid_path(path) != 1) {
        return badcommandNotAlphanum();
    }

    if (fs_create(path, DIRECTORY_T) != 0) {
        return badCommandErrorOccurred();
    }

    return 0;
}
