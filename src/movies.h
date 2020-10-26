#ifndef MOVIEDB_MOVIES_H
#define MOVIEDB_MOVIES_H 1

#include "error.h"
#include "id.h"
#include "csv/movie.h"

struct movie {
    moviedb_id id;
    char const *title;
    char const *genres;
};

struct movies_table {
    struct movie **entries;
    size_t length;
    size_t capacity;
};

void movies_init(
        struct movies_table *restrict table,
        struct error *error);

void movies_insert(
        struct movies_table *restrict table,
        struct movie_csv_row *restrict csv_row,
        struct error *error);

struct movie *movies_search(
        struct movies_table const *restrict table,
        moviedb_id movieid);

#endif
