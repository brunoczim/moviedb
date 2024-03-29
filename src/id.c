#include "id.h"
#include "alloc.h"
#include <string.h>

moviedb_id_t moviedb_id_parse(
        char const *restrict string,
        struct error *restrict error)
{
    size_t i = 0;
    moviedb_id_t id = 0;
    char *error_string;

    while (string[i] != 0 && error->code == error_none) {
        if (id * 10 >= id && string[i] >= '0' && string[i] <= '9') {
            /* Accounts the digit. */
            id *= 10;
            id += string[i] - '0';
            i++;
        } else {
            if (error->code == error_none) {
                error_string = moviedb_alloc(
                        sizeof(*error_string),
                        strlen(string) + 1,
                        error);

                if (error->code == error_none) {
                    /* Copies the input string to an error. */
                    strcpy(error_string, string);
                    error_set_code(error, error_id);
                    error->data.id.has_line = false;
                    error->data.id.string = error_string;
                    error->data.id.free_string = true;
                }
            }
        }
    }

    /* Empty string is an error. */
    if (i == 0 && error->code == error_none) {
        error_set_code(error, error_id);
        error->data.id.has_line = false;
        error->data.id.string = "";
        error->data.id.free_string = false;
    }

    return id;
}

size_t moviedb_id_to_str(
        moviedb_id_t id,
        char *restrict buffer,
        size_t buf_size)
{
    size_t start = buf_size;

    if (start > 0) {
        start--;
        buffer[start] = 0;

        /* Write the number digits starting backwards. */
        do {
            if (start > 0) {
                start--;
                buffer[start] = '0' + id % 10;
                id /= 10;
            }
        } while (start > 0 && id > 0);
    }

    return start;
}

extern inline moviedb_hash_t moviedb_id_hash(moviedb_id_t id);
