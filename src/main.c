#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "error.h"
#include "alloc.h"
#include "strbuf.h"
#include "io.h"
#include "movie.h"
#include "trie.h"

#define IO_BUF_SIZE 0x10000

void load_all(struct trie_node *restrict root, struct error *error);

void load_movies(
        struct trie_node *restrict root,
        char *buffer,
        struct error *error);

int main(int argc, char const *argv[])
{
    int exit_code = 0;
    struct error error;
    struct trie_node root;

    error_init(&error);
    trie_root_init(&root);
    load_all(&root, &error);

    if (error.code != error_none) {
        error_print(&error);
    }

    trie_destroy(&root);
    error_destroy(&error);

    return exit_code;
}

void load_all(struct trie_node *restrict root, struct error *error)
{
    clock_t then, now;
    double millis;
    char *buffer;

    puts("Loading data...");

    then = clock();

    buffer = moviedb_alloc(IO_BUF_SIZE, error);
    if (error->code == error_none)  {
        load_movies(root, buffer, error);
        free(buffer);
    }
    now = clock();

    millis = (now - then) / (CLOCKS_PER_SEC / 1000.0);
    printf("Data loaded in %.3lf milliseconds\n", millis);
}

void load_movies(
        struct trie_node *restrict root,
        char *buffer,
        struct error *error)
{
    char const *path = "movie.csv";
    FILE *file;
    struct movie_parser parser;
    struct strbuf buf;
    struct movie_csv_row row;
    bool has_data = true;

    file = input_file_open(path, error);

    if (error->code == error_none) {
        input_file_setbuf(file, buffer, IO_BUF_SIZE, error);
    }

    if (error->code == error_none) {
        strbuf_init(&buf);
        movie_parser_init(&parser, file, &buf, error);

        while (has_data) {
            has_data= movie_parse_row(&parser, &buf, &row, error);
            if (has_data) {
                trie_insert(root, row.title, row.id, error);
                has_data = error->code == error_none;
                movie_destroy_row(&row);
            }
        }

        strbuf_destroy(&buf);
        input_file_close(file);
    } 

    if (error->code != error_none) {
        error_set_context(error, path, false);
    }
}

