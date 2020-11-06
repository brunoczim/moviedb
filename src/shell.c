#include "io.h"
#include "alloc.h"
#include "shell.h"
#include <string.h>
#include <inttypes.h>

#define MIN_RATINGS 1000

/**
 * Data shared by shell functions.
 */
struct shell {
    /**
     * An immutable pointer to the database.
     */
    struct database const *restrict database;
    /**
     * Buffer used by the shell to read lines.
     */
    struct strbuf *restrict buf;
    /**
     * Last read character.
     */
    int curr_ch;
};

/**
 * Skips whitespace in the user input.
 */
static void skip_whitespace(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Reads and runs a command entered by the user. Returns whether the shell
 * should still execute.
 */
static bool run_cmd(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Reads a movie title prefix entered by the user and runs a search for movies
 * with that prefix in their names. Returns whether the shell should still
 * execute.
 */
static bool run_movie(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Runs the user command. The command searches for the ratings of a user.
 * Returns whether the shell should still execute.
 */
static bool run_user(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Runs the topN command. The command finds N movies with the best ratings of a
 * given genre. Returns whether the shell should still execute.
 */
static bool run_topn(
        struct shell *restrict shell,
        struct error *restrict error);

static bool run_tags(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Reads the operation name entered by the user such as "movie" or "exit.
 * Returns whether the shell should still execute.
 */
static bool read_op(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Reads the single argument of an operation.
 */
static void read_single_arg(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Reads a quoted argument. I.e. an argument of the form 'abc'. Escapes \\, \n
 * \" and \'.
 */
static void read_quoted_arg(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Reads the end of a command. Sets an error if there is more arguments.
 */
static void read_end(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Discards an entire line from the terminal input.
 */
static void discard_line(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Prints a help message to the user, showing all operations.
 */
static void print_help(void);

void shell_run(struct database const *restrict database,
        struct strbuf *restrict buf,
        struct error *restrict error)
{
    bool read = true;
    struct shell shell;
    shell.database = database;
    shell.buf = buf;

    while (read && error->code == error_none) {
        printf("$ ");
        fflush(stdout);
        shell.curr_ch = input_file_read(stdin, error);
        read = run_cmd(&shell, error);
    }
}

static void skip_whitespace(
        struct shell *restrict shell,
        struct error *restrict error)
{
    bool delimiter = false;

    do {
        switch (shell->curr_ch) {
            case ' ':
                shell->curr_ch = input_file_read(stdin, error);
                break;
            default:
                delimiter = true;
                break;
        }
    } while (!delimiter && error->code == error_none);
}

static bool run_cmd(
        struct shell *restrict shell,
        struct error *restrict error)
{
    skip_whitespace(shell, error);
    if (!read_op(shell, error)) {
        puts("exit");
        return false;
    }

    strbuf_make_cstr(shell->buf, error);

    if (error->code != error_none || strcmp(shell->buf->ptr, "exit") == 0) {
        return false;
    }

    if (strcmp(shell->buf->ptr, "movie") == 0) {
        run_movie(shell, error);
    } else if (strcmp(shell->buf->ptr, "user") == 0) {
        run_user(shell, error);
    } else if (strcmp(shell->buf->ptr, "tags") == 0) {
        run_tags(shell, error);
    } else if (strncmp(shell->buf->ptr, "top", sizeof("top") - 1) == 0) {
        run_topn(shell, error);
    } else {
        print_help();
    }

    return true;
}

static bool run_movie(
        struct shell *restrict shell,
        struct error *restrict error)
{
    struct movie_query_buf query_buf;

    read_single_arg(shell, error);

    if (error->code == error_none) {
        strbuf_make_cstr(shell->buf, error);
    }

    if (error->code == error_none) {
        movie_query_init(&query_buf);
        movie_query(shell->database, shell->buf->ptr, &query_buf, error);
    }

    if (error->code == error_none) {
        movie_query_print(&query_buf);
        movie_query_destroy(&query_buf);
    }

    return error->code == error_none;
}

static bool run_user(
        struct shell *restrict shell,
        struct error *restrict error)
{
    struct user_query_iter query_iter;
    db_id_t userid = 0;

    read_single_arg(shell, error);

    if (error->code == error_none) {
        strbuf_make_cstr(shell->buf, error);
    }

    if (error->code == error_none) {
        userid = db_id_parse(shell->buf->ptr, error);
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

static bool run_topn(
        struct shell *restrict shell,
        struct error *restrict error)
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
        read_quoted_arg(shell, error);
    }

    if (error->code == error_none) {
        read_end(shell, error);
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

static bool run_tags(
        struct shell *restrict shell,
        struct error *restrict error)
{
    struct tags_query_input query_input;
    struct tags_query_buf query_buf;

    tags_query_input_init(&query_input, 2, error);

    while (error->code == error_none) {
        read_quoted_arg(shell, error);
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

static bool read_op(
        struct shell *restrict shell,
        struct error *restrict error)
{
    bool delimiter;

    delimiter = false;
    shell->buf->length = 0;
    skip_whitespace(shell, error);

    while (!delimiter && error->code == error_none) {
        switch (shell->curr_ch) {
            case '\n':
            case ' ':
            case EOF:
                delimiter = true;
                break;
            default:
                strbuf_push(shell->buf, shell->curr_ch, error);
                if (error->code == error_none) {
                    shell->curr_ch = input_file_read(stdin, error);
                }
                break;
        }
    }

    if (error->code != error_none) {
        return false;
    }
    return shell->buf->length > 0 || shell->curr_ch != EOF;
}

static void read_single_arg(
        struct shell *restrict shell,
        struct error *restrict error)
{
    bool delimiter;

    shell->buf->length = 0;
    skip_whitespace(shell, error);
    delimiter = false;

    while (!delimiter && error->code == error_none) {
        switch (shell->curr_ch) {
            case '\n':
            case EOF:
                delimiter = true;
                break;
            default:
                strbuf_push(shell->buf, shell->curr_ch, error);
                if (error->code == error_none) {
                    shell->curr_ch = input_file_read(stdin, error);
                }
                break;
        }
    }
}

static void read_quoted_arg(
        struct shell *restrict shell,
        struct error *restrict error)
{
    char bad_quote;
    char quote = 0;
    bool delimiter = false;
    bool escape = false;

    skip_whitespace(shell, error);

    if (error->code == error_none) {
        shell->buf->length = 0;

        delimiter = false;

        switch (shell->curr_ch) {
            case EOF:
            case '\n':
                error_set_code(error, error_expected_arg);
                break;
            case '"':
            case '\'':
                quote = shell->curr_ch;
                shell->curr_ch = input_file_read(stdin, error);
                break;
            default:
                bad_quote = shell->curr_ch;
                discard_line(shell, error);
                if (error->code == error_none) {
                    error_set_code(error, error_bad_quote);
                    error->data.bad_quote.found = bad_quote;
                }
                break;
        }

        while (!delimiter && error->code == error_none) {
            if (shell->curr_ch == EOF || shell->curr_ch == '\n') {
                discard_line(shell, error);

                if (error->code == error_none) {
                    strbuf_make_cstr(shell->buf, error);
                }

                if (error->code == error_none) {
                    error_set_code(error, error_open_quote);
                    error->data.open_quote.string = shell->buf->ptr;
                    error->data.open_quote.free_string = false;
                }
            } else if (escape) {
                if (shell->curr_ch == 'n') {
                    strbuf_push(shell->buf, '\n', error);
                } else {
                    strbuf_push(shell->buf, shell->curr_ch, error);
                }
                escape = false;
            } else if (shell->curr_ch == quote) {
                delimiter = true;
            } else if (shell->curr_ch == '\\') {
                escape = true;
            } else {
                strbuf_push(shell->buf, shell->curr_ch, error);
            }

            if (error->code == error_none) {
                shell->curr_ch = input_file_read(stdin, error);
            }
        }
    }
}

static void read_end(
        struct shell *restrict shell,
        struct error *restrict error)
{
    skip_whitespace(shell, error);

    if (error->code == error_none) {
        switch (shell->curr_ch) {
            case EOF:
            case '\n':
                break;
            default:
                error_set_code(error, error_expected_end);
                break;
        }
    }
}

static void discard_line(
        struct shell *restrict shell,
        struct error *restrict error)
{
    bool end = false;
    while (error->code == error_none && !end) {
        end = shell->curr_ch == EOF || shell->curr_ch == '\n';
        if (!end) {
            shell->curr_ch = input_file_read(stdin, error);
        }
    }
}

static void print_help(void)
{
    char const *head, *movie, *user, *topn, *tags, *exit;

    head  = "Commands available:\n";
    movie = "    $ movie <prefix or title>       searches movies\n";
    user  = "    $ user <user ID>                finds user's ratings\n";
    topn  = "    $ top<N> '<genre>'              lists genre's N best movies\n";
    tags  = "    $ tags <'list' 'of' 'tags'>     lists movies with all tags \n";
    exit  = "    $ exit                          exits\n";

    fputs(head, stderr);
    fputs(movie, stderr);
    fputs(user, stderr);
    fputs(topn, stderr);
    fputs(tags, stderr);
    fputs(exit, stderr);
}
