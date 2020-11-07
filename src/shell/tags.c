#include "tags.h"
#include "../query.h"

bool shell_run_tags(struct shell *restrict shell, struct error *restrict error)
{
    struct tags_query_input query_input;
    struct tags_query_buf query_buf;

    tags_query_input_init(&query_input, 2, error);

    while (error->code == error_none) {
        shell_read_quoted_arg(shell, error);
        if (error->code == error_none) {
            strbuf_make_cstr(shell->buf, error);
        }
        if (error->code == error_none) {
            tags_query_input_add(&query_input,
                    shell->database,
                    shell->buf->ptr,
                    error);
        }
    }

    if (error->code == error_expected_arg) {
        error_set_code(error, error_none);
    }

    switch (error->code) {
        case error_none:
            tags_query_init(&query_buf);
            tags_query(shell->database, &query_input, &query_buf, error);        
            if (error->code == error_none) {
                tags_query_print(&query_buf);
            }
            tags_query_destroy(&query_buf);
            break;

        case error_open_quote:
        case error_bad_quote:
            error_print(error);
            error_set_code(error, error_none);
            break;

        default:
            break;
    }

    tags_query_input_destroy(&query_input);

    return error->code == error_none;
}
