#include "topn.h"
#include "../query.h"
#include <inttypes.h>

#define MIN_RATINGS 1000

bool shell_run_topn(struct shell *restrict shell, struct error *restrict error)
{
    uintmax_t converted;
    size_t count;
    char *start, *end;
    struct topn_query_buf query_buf;

    start = shell->buf->ptr + (sizeof("top") - 1);

    converted = strtoumax(start, &end, 10);

    if (*end != 0) {
        error_set_code(error, error_topn_count);
        error->data.topn_count.string = start;
        error->data.topn_count.free_string = false;
    }

    if (error->code == error_none) {
        shell_read_quoted_arg(shell, error);
    }

    if (error->code == error_none) {
        shell_read_end(shell, error);
    }

    if (error->code == error_none) {
        strbuf_make_cstr(shell->buf, error);
    }

    if (error->code == error_none) {
        if (converted > shell->database->movies.length) {
            count = shell->database->movies.length;
        } else {
            count = converted;
        }

        topn_query_init(&query_buf, count, error);
    }

    switch (error->code) {
        case error_none:
            topn_query(shell->database,
                    shell->buf->ptr,
                    MIN_RATINGS,
                    &query_buf);

            topn_query_print(&query_buf);
            topn_query_destroy(&query_buf);
            break;

        case error_open_quote:
        case error_expected_arg:
        case error_bad_quote:
        case error_topn_count:
            error_print(error);
            error_set_code(error, error_none);
            break;

        default:
            break;
    }

    return error->code == error_none;
}

