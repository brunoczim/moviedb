#include "id.h"
#include "alloc.h"
#include <string.h>

db_id_t db_id_parse(char const *restrict string, struct error *restrict error)
{
    size_t i = 0;
    db_id_t id = 0;
    char *error_string;

    while (string[i] != 0 && error->code == error_none) {
        if (string[i] >= '0' && string[i] <= '9') {
            id *= 10;
            id += string[i] - '0';
            i++;
        } else {
            if (error->code == error_none) {
                error_string = db_alloc(strlen(string) + 1, error);
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

extern inline db_hash_t db_id_hash(db_id_t id);
