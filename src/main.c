#include <stdio.h>
#include <stdbool.h>
#include "error.h"
#include "io.h"
#include "strbuf.h"
#include "csv.h"
#include "trie.h"

void read_rating(struct trie_node *restrict root, struct error *error)
{
    char const *path = "rating.csv";
    FILE *file;
    struct csv_parser parser;
    struct strbuf field;

    file = input_file_open(path, error);

    if (error->code == error_none) {
        csv_parser_init(&parser, file);
        strbuf_init(&field);

        strbuf_destroy(&field);
        input_file_close(file);
    } else {
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
    read_rating(&root, &error);

    if (error.code != error_none) {
        error_print(&error);
    }

    trie_destroy(&root);
    error_destroy(&error);

    return exit_code;
}
