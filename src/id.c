#include "id.h"

moviedb_id moviedb_id_parse(struct strbuf *restrict buf, struct error *error)
{
    size_t i = 0;
    moviedb_id id = 0;
    char const *error_string;

    while (i < buf->length && error->code == error_none) {
        if (buf->ptr[i] >= '0' && buf->ptr[i] <= '9') {
            id *= 10;
            id += buf->ptr[i] - '0';
            i++;
        } else {
            error_string = strbuf_make_cstr(buf, error);
            if (error->code == error_none) {
                error_set_code(error, error_id);
                error->data.id.has_line = false;
                error->data.id.string = error_string;
                error->data.id.free_string = true;
            }
        }
    }

    if (buf->length == 0) {
        error_set_code(error, error_id);
        error->data.id.has_line = false;
        error->data.id.string = "";
        error->data.id.free_string = false;
    }

    return id;
}
