#include "movie.h"
#include "../query.h"

bool shell_run_movie(struct shell *restrict shell, struct error *restrict error)
{
    struct movie_query_buf query_buf;

    /* Reads the argument that takes the whole rest of the line. */
    shell_read_single_arg(shell, error);

    if (error->code == error_none) {
        strbuf_make_cstr(shell->buf, error);
    }

    if (error->code == error_none) {
        /* Performs the query. */
        movie_query_init(&query_buf);
        movie_query(shell->database, shell->buf->ptr, &query_buf, error);
    }

    if (error->code == error_none) {
        movie_query_print(&query_buf);
        movie_query_destroy(&query_buf);
    }

    return error->code == error_none;
}
