#include "io.h"
#include "alloc.h"
#include "shell/movie.h"
#include "shell/user.h"
#include "shell/topn.h"
#include "shell/tags.h"
#include <string.h>

void shell_run(struct database const *restrict database,
        struct strbuf *restrict buf,
        struct error *restrict error)
{
    struct shell shell;
    bool read = true;
    shell.database = database;
    shell.buf = buf;

    while (read && error->code == error_none) {
        printf("$ ");
        fflush(stdout);
        shell.curr_ch = input_file_read(stdin, error);
        read = shell_run_cmd(&shell, error);
    }
}

void shell_skip_whitespace(
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

bool shell_run_cmd(struct shell *restrict shell, struct error *restrict error)
{
    shell_skip_whitespace(shell, error);
    if (!shell_read_op(shell, error)) {
        puts("exit");
        return false;
    }

    strbuf_make_cstr(shell->buf, error);

    if (error->code != error_none || strcmp(shell->buf->ptr, "exit") == 0) {
        return false;
    }

    if (strcmp(shell->buf->ptr, "movie") == 0) {
        shell_run_movie(shell, error);
    } else if (strcmp(shell->buf->ptr, "user") == 0) {
        shell_run_user(shell, error);
    } else if (strcmp(shell->buf->ptr, "tags") == 0) {
        shell_run_tags(shell, error);
    } else if (strncmp(shell->buf->ptr, "top", sizeof("top") - 1) == 0) {
        shell_run_topn(shell, error);
    } else {
        shell_print_help();
    }

    return true;
}

bool shell_read_op(struct shell *restrict shell, struct error *restrict error)
{
    bool delimiter;

    delimiter = false;
    shell->buf->length = 0;
    shell_skip_whitespace(shell, error);

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

void shell_read_single_arg(
        struct shell *restrict shell,
        struct error *restrict error)
{
    bool delimiter;

    shell->buf->length = 0;
    shell_skip_whitespace(shell, error);
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

void shell_read_quoted_arg(
        struct shell *restrict shell,
        struct error *restrict error)
{
    char bad_quote;
    char quote = 0;
    bool delimiter = false;
    bool escape = false;

    shell_skip_whitespace(shell, error);

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
                shell_discard_line(shell, error);
                if (error->code == error_none) {
                    error_set_code(error, error_bad_quote);
                    error->data.bad_quote.found = bad_quote;
                }
                break;
        }

        while (!delimiter && error->code == error_none) {
            if (shell->curr_ch == EOF || shell->curr_ch == '\n') {
                shell_discard_line(shell, error);

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

void shell_read_end(
        struct shell *restrict shell,
        struct error *restrict error)
{
    shell_skip_whitespace(shell, error);

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

void shell_discard_line(
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

void shell_print_help(void)
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
