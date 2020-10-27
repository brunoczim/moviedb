#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "../alloc.h"
#include "../movies.h"
#include "../error.h"

/** 
 * Tests movies hash table implementation.
 */

void insert(
        struct movies_table *restrict table,
        moviedb_id id,
        char const *title,
        char const *genres,
        struct error *error)
{
    printf("Inserting %s\n", title);

    char *heap_title, *heap_genres;
    struct movie_csv_row row;

    row.id = id;

    heap_title = moviedb_alloc(strlen(title) + 1, error);
    assert(error->code == error_none);
    strcpy(heap_title, title);
    row.title = heap_title;

    heap_genres = moviedb_alloc(strlen(genres) + 1, error);
    assert(error->code == error_none);
    strcpy(heap_genres, genres);
    row.genres = heap_genres;

    movies_insert(table, &row, error);
}

int main(int argc, char const *argv[])
{
    struct error error;
    struct movies_table table;

    error_init(&error);

    movies_init(&table, 5, &error);
    assert(error.code == error_none);
    assert(table.length == 0);
    assert(table.capacity == 5);

    insert(&table, 123, "Banana Movie", "action|comedy", &error);
    assert(error.code == error_none);
    assert(table.length == 1);
    assert(table.capacity == 5);

    insert(&table, 456, "Apple Film", "comedy|drama", &error);
    assert(error.code == error_none);
    assert(table.length == 2);
    assert(table.capacity == 5);

    insert(&table, 789, "Pelicula de la Naranja", "action|drama", &error);
    assert(table.length == 3);
    assert(table.capacity == 11);

    insert(&table, 456, "Bad Duplicate", "fiction", &error);
    assert(error.code == error_dup_movie_id);
    assert(table.length == 3);
    assert(table.capacity == 11);

    movies_destroy(&table);
    error_destroy(&error);

    puts("Ok");

    return 0;
}
