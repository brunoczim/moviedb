#include "io.h"
#include "alloc.h"
#include "shell.h"

struct shell {
    struct trie_node const *trie_root;
    struct strbuf *buf;
    int curr_ch;
};

static void skip_whitespace(struct shell *restrict shell, struct error *error);

static bool run_cmd(struct shell *restrict shell, struct error *error);

static bool run_movie(struct shell *restrict shell, struct error *error);

static bool read_op(struct shell *restrict shell, struct error *error);

static void print_help(void);

void shell_run(struct trie_node const *trie_root,
        struct strbuf *buf,
        struct error *error)
{
    bool read = true;
    struct shell shell;
    shell.trie_root = trie_root;
    shell.buf = buf;
    shell.curr_ch = input_file_read(stdin, error);

    while (read && error->code == error_none) {
        printf("$ ");
        fflush(stdout);
        read = run_cmd(&shell, error);
    }
}

static void skip_whitespace(struct shell *restrict shell, struct error *error)
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

static bool run_cmd(struct shell *restrict shell, struct error *error)
{
    struct strref buf_ref;
    skip_whitespace(shell, error);
    if (!read_op(shell, error)) {
        return false;
    }
    strbuf_as_ref(shell->buf, &buf_ref);
    if (strref_icmp_cstr(&buf_ref, "exit") == 0) {
        return false;
    }

    if (strref_icmp_cstr(&buf_ref, "movie") == 0) {
        run_movie(shell, error);
    } else {
        print_help();
    }

    return true;
}

static bool run_movie(struct shell *restrict shell, struct error *error)
{
    bool delimiter;
    bool found;
    char *title;
    moviedb_id movieid;

    shell->buf->length = 0;

    do {
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
    } while (!delimiter && error->code == error_none);

    if (error->code == error_none) {
        title = strbuf_make_cstr(shell->buf, error);
    }

    if (error->code == error_none) {
        found = trie_search(shell->trie_root, title, &movieid);
        moviedb_free(title);
        if (found) {
            printf("%lu\n", movieid);
        }
    }

    return error->code == error_none;
}

static bool read_op(struct shell *restrict shell, struct error *error)
{
    bool delimiter;

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

    return shell->buf->length > 0 && error->code == error_none;
}

static void print_help(void)
{
    fputs("Commands available:\n", stderr);
    fputs("    $ movie <prefix or title>        searches movie\n", stderr);
    fputs("    $ exit                           exits\n", stderr);
}
