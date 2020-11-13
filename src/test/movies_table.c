#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "../alloc.h"
#include "../movies.h"
#include "../error.h"

/** 
 * Tests movies hash table implementation.
 */

void insert(
        struct movies_table *restrict table,
        moviedb_id_t id,
        char const *restrict title,
        char const *restrict genres,
        struct error *restrict error)
{
    printf("Inserting %s\n", title);

    char *heap_title, *heap_genres;
    struct movie_csv_row row;

    row.id = id;

    heap_title = moviedb_alloc(sizeof(*heap_title), strlen(title) + 1, error);
    assert(error->code == error_none);
    strcpy(heap_title, title);
    row.title = heap_title;

    heap_genres = moviedb_alloc(
            sizeof(*heap_genres),
            strlen(genres) + 1,
            error);
    assert(error->code == error_none);
    strcpy(heap_genres, genres);
    row.genres = heap_genres;

    movies_insert(table, &row, error);

    if (error->code != error_none) {
        moviedb_free(heap_title);
        moviedb_free(heap_genres);
    }
}

int main(int argc, char const *argv[])
{
    struct error error;
    struct movie const *movie;
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

    movies_add_rating(&table, 456, 2.0);
    movies_add_rating(&table, 456, 3.0);

    movie = movies_search(&table, 456);
    assert(movie != NULL);
    assert(movie->id == 456);
    assert(strcmp(movie->title, "Apple Film") == 0);
    assert(strcmp(movie->genres, "comedy|drama") == 0);
    assert(fabs(movie->mean_rating - 2.5) < 0.000001);
    assert(movie->ratings == 2);

    movie = movies_search(&table, 123);
    assert(movie != NULL);
    assert(movie->id == 123);
    assert(strcmp(movie->title, "Banana Movie") == 0);
    assert(strcmp(movie->genres, "action|comedy") == 0);
    assert(fabs(movie->mean_rating) < 0.000001);
    assert(movie->ratings == 0);

    movies_destroy(&table);
    error_destroy(&error);

    puts("Ok");

    return 0;
}
