#include <stdio.h>
#include <stdbool.h>
#include "error.h"
#include "io.h"
#include "strbuf.h"
#include "movie.h"
#include "trie.h"

void read_movies(struct trie_node *restrict root, struct error *error)
{
    char const *path = "movie.csv";
    FILE *file;
    struct movie_parser parser;
    struct strbuf buf;
    struct movie_csv_row row;
    bool has_data = true;

    file = input_file_open(path, error);

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

int main(int argc, char const *argv[])
{
    int exit_code = 0;
    struct error error;
    struct trie_node root;

    error_init(&error);
    trie_root_init(&root);
    read_movies(&root, &error);

    if (error.code != error_none) {
        error_print(&error);
    }

    trie_destroy(&root);
    error_destroy(&error);

    return exit_code;
}
