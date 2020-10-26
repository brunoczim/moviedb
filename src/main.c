#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "error.h"
#include "alloc.h"
#include "strbuf.h"
#include "io.h"
#include "csv/movie.h"
#include "trie.h"
#include "shell.h"

#define IO_BUF_SIZE 0x10000

void load_all(
        struct trie_node *restrict root,
        struct strbuf *restrict buf,
        struct error *error);

void load_movies(
        struct trie_node *restrict root,
        struct strbuf *restrict buf,
        char *file_buf,
        struct error *error);

int main(int argc, char const *argv[])
{
    int exit_code = 0;
    struct error error;
    struct trie_node root;
    struct strbuf buf;

    error_init(&error);
    strbuf_init(&buf);
    trie_root_init(&root);
    load_all(&root, &buf, &error);

    if (error.code == error_none) {
        shell_run(&root, &buf, &error);
    }

    if (error.code != error_none) {
        error_print(&error);
    }

    trie_destroy(&root);
    strbuf_destroy(&buf);
    error_destroy(&error);

    return exit_code;
}

void load_all(
        struct trie_node *restrict root,
        struct strbuf *restrict buf,
        struct error *error)
{
    clock_t then, now;
    double millis;
    char *file_buf;

    puts("Loading data...");

    then = clock();

    file_buf = moviedb_alloc(IO_BUF_SIZE, error);
    if (error->code == error_none)  {
        load_movies(root, buf, file_buf, error);
        free(file_buf);
    }
    now = clock();

    millis = (now - then) / (CLOCKS_PER_SEC / 1000.0);
    printf("Data loaded in %.3lf milliseconds\n", millis);
}

void load_movies(
        struct trie_node *restrict root,
        struct strbuf *restrict buf,
        char *file_buf,
        struct error *error)
{
    char const *path = "movie.csv";
    FILE *file;
    struct movie_parser parser;
    struct movie_csv_row row;
    bool has_data = true;

    file = input_file_open(path, error);

    if (error->code == error_none) {
        input_file_setbuf(file, file_buf, IO_BUF_SIZE, error);
    }

    if (error->code == error_none) {
        movie_parser_init(&parser, file, buf, error);

        while (has_data) {
            has_data= movie_parse_row(&parser, buf, &row, error);
            if (has_data) {
                trie_insert(root, row.title, row.id, error);
                has_data = error->code == error_none;
                movie_destroy_row(&row);
            }
        }

        input_file_close(file);
    } 

    if (error->code != error_none) {
        error_set_context(error, path, false);
    }
}

