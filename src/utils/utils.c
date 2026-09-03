#include <ctype.h>
#include <stdlib.h>

/*
 * Checks if a path is valid. Used for checking file and directory paths.
 * Alphanumeric characters and underscores are allowed.
 *
 * Returns:
 * - 0 if not alphanumeric
 * - 1 if alphanumeric
 */
int is_valid_path(char* path) {
    if (path == NULL) {
        return 0;
    }

    int ix = 0;
    while (path[ix] != '\0') {
        if (isalnum(path[ix]) == 0 && path[ix] != '_' && path[ix] != '/') {
            return 0;
        }

        ix++;
    }

    return 1;
}

/*
 * performs ceiling division using formula:
 *
 * ceil(n / k) = (n + k - 1) / k
 *
 * returns an integer, hence the suffix `i`.
 */
int ceili(int n, int k) { return (n + k - 1) / k; }
