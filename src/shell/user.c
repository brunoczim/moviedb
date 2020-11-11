#include "user.h"
#include "../query.h"

bool shell_run_user(struct shell *restrict shell, struct error *restrict error)
{
    struct user_query_iter query_iter;
    moviedb_id_t userid = 0;

    shell_read_single_arg(shell, error);

    if (error->code == error_none) {
        strbuf_make_cstr(shell->buf, error);
    }

    if (error->code == error_none) {
        userid = moviedb_id_parse(shell->buf->ptr, error);
    }

    if (error->code == error_id) {
        error_print(error);
        error_set_code(error, error_none);
    } else {
        user_query_init(&query_iter, shell->database, userid);
        user_query_print(&query_iter);
    }

    return error->code == error_none;
}
