#include "io.h"
#include "alloc.h"
#include "shell.h"
#include <string.h>

/**
 * Data shared by shell functions.
 */
struct shell {
    /**
     * The trie mapping title -> movieid.
     */
    struct trie_node const *restrict trie_root;
    /**
     * The hash table mapping movieid -> movie.
     */
    struct movies_table const *restrict movies;
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
 * Prints a help message to the user, showing all operations.
 */
static void print_help(void);

void shell_run(struct trie_node const *restrict trie_root,
        struct movies_table const *restrict movies,
        struct strbuf *restrict buf,
        struct error *restrict error)
{
    bool read = true;
    struct shell shell;
    shell.trie_root = trie_root;
    shell.movies = movies;
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
    } else {
        print_help();
    }

    return true;
}

static bool run_movie(
        struct shell *restrict shell,
        struct error *restrict error)
{
    char *prefix;
    struct trie_iter iter;
    struct movie const *movie;
    db_id_t movieid;

    read_single_arg(shell, error);

    if (error->code == error_none) {
        prefix = strbuf_copy_cstr(shell->buf, error);
    }

    if (error->code == error_none) {
        trie_search_prefix(shell->trie_root, prefix, &iter);
        db_free(prefix);
        while (trie_next_movie(&iter, &movieid, error)) {
            movie = movies_search(shell->movies, movieid);
            if (movie != NULL) {
                movie_print(movie);
            }
        }
        trie_iter_destroy(&iter);
    }

    return error->code == error_none;
}

static bool read_op(
        struct shell *restrict shell,
        struct error *restrict error)
{
    bool delimiter;

    delimiter = false;
    shell->buf->length = 0;

    do {
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
    } while (!delimiter && error->code == error_none);

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
    
    skip_whitespace(shell, error);

    shell->buf->length = 0;
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

static void print_help(void)
{
    fputs("Commands available:\n", stderr);
    fputs("    $ movie <prefix or title>        searches movie\n", stderr);
    fputs("    $ exit                           exits\n", stderr);
}
