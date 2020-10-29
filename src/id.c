#include "id.h"
#include "alloc.h"
#include <string.h>

moviedb_id moviedb_id_parse(
        char const *restrict string,
        struct error *restrict error)
{
    size_t i = 0;
    moviedb_id id = 0;
    char *error_string;

    while (string[i] != 0 && error->code == error_none) {
        if (string[i] >= '0' && string[i] <= '9') {
            id *= 10;
            id += string[i] - '0';
            i++;
        } else {
            if (error->code == error_none) {
                error_string = moviedb_alloc(strlen(string) + 1, error);
                if (error->code == error_none) {
                    strcpy(error_string, string);
                    error_set_code(error, error_id);
                    error->data.id.has_line = false;
                    error->data.id.string = error_string;
                    error->data.id.free_string = true;
                }
            }
        }
    }

    if (i == 0 && error->code == error_none) {
        error_set_code(error, error_id);
        error->data.id.has_line = false;
        error->data.id.string = "";
        error->data.id.free_string = false;
    }

    return id;
}

uint_fast64_t moviedb_id_hash(moviedb_id id)
{
    uint_fast64_t hash = id;

    /* xorshift algorithm */
    hash ^= hash << 13;
    hash ^= hash >> 7;
    hash ^= hash << 17;

    /* large integer prime multiplication. */
    hash *= 0x3F1F06B16A65D581ull;

    /* xorshift again */
    hash ^= hash << 13;
    hash ^= hash >> 7;
    hash ^= hash << 17;

    /* multiplication by prime again. */
    hash *= 0x452537355C0164DDull;

    return hash;
}
