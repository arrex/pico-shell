#include "../file_system/file_system.h"
#include "../utils/badcommand.h"
#include "../utils/utils.h"

int cd(char* dirname) {
    if (is_valid_path(dirname) != 1) {
        return badcommandNotAlphanum();
    }

    if (fs_chdir(dirname) != 0) {
        return badcommandDirDoesNotExist();
    }

    return 0;
}
