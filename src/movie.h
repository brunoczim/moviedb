#ifndef MOVIEDB_MOVIE_H
#define MOVIEDB_MOVIE_H 1

#include "csv.h"
#include "strbuf.h"
#include "io.h"
#include "id.h"

struct movie_csv_row {
    moviedb_id id;
    char const *title;
    char const *genres;
};

struct movie_parser {
    struct csv_parser csv_parser;
    unsigned char id_column;
    unsigned char title_column;
    unsigned char genres_column;
};

void movie_parser_init(
        struct movie_parser *restrict parser,
        FILE *file,
        struct strbuf *buf,
        struct error *error);

bool movie_parse_row(
        struct movie_parser *restrict parser,
        struct strbuf *buf,
        struct movie_csv_row *row_out,
        struct error *error);

void movie_destroy_row(struct movie_csv_row *row);

#endif
