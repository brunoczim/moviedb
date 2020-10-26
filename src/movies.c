#include "movies.h"

void movies_init(
        struct movies_table *restrict table,
        struct error *error)
{
}

void movies_insert(
        struct movies_table *restrict table,
        struct movie_csv_row *restrict csv_row,
        struct error *error)
{
}

struct movie *movies_search(
        struct movies_table const *restrict table,
        moviedb_id movieid)
{
    return NULL;
}
