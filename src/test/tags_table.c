#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "../alloc.h"
#include "../tags.h"
#include "../error.h"

/** 
 * Tests tags hash table implementation.
 */

void insert(
        struct tags_table *restrict table,
        char const *restrict name,
        moviedb_id_t movie,
        struct error *restrict error)
{
    printf("Inserting %s\n", name);

    char *heap_name;
    struct tag_csv_row row;

    row.movieid = movie;

    heap_name = moviedb_alloc(sizeof(*heap_name), strlen(name) + 1, error);
    assert(error->code == error_none);
    strcpy(heap_name, name);
    row.name = heap_name;

    tags_insert(table, &row, error);

    if (error->code != error_none) {
        moviedb_free(heap_name);
    }
}

int main(int argc, char const *argv[])
{
    struct error error;
    struct tag const *tag;
    struct tags_table table;
    struct tag_movies_iter iter;
    moviedb_id_t movieid;
    bool found88 = false, found90 = false, found92 = false;

    error_init(&error);

    tags_init(&table, 5, &error);
    assert(error.code == error_none);
    assert(table.length == 0);
    assert(table.capacity == 5);

    insert(&table, "good", 90, &error);
    assert(error.code == error_none);
    assert(table.length == 1);
    assert(table.capacity == 5);

    tag = tags_search(&table, "good");
    assert(tag != NULL);
    assert(strcmp(tag->name, "good") == 0);

    insert(&table, "bad", 92, &error);
    assert(error.code == error_none);
    assert(table.length == 2);
    assert(table.capacity == 5);

    insert(&table, "average", 88, &error);
    assert(error.code == error_none);
    assert(table.length == 3);
    assert(table.capacity == 11);

    tag = tags_search(&table, "good");
    assert(tag != NULL);
    assert(strcmp(tag->name, "good") == 0);

    tag = tags_search(&table, "bad");
    assert(tag != NULL);
    assert(strcmp(tag->name, "bad") == 0);

    insert(&table, "good", 92, &error);
    assert(error.code == error_none);
    assert(table.length == 3);
    assert(table.capacity == 11);

    insert(&table, "good", 88, &error);
    assert(error.code == error_none);
    assert(table.length == 3);
    assert(table.capacity == 11);

    tag = tags_search(&table, "bad");
    assert(tag != NULL);
    assert(strcmp(tag->name, "bad") == 0);
    assert(tag->movies.length == 1);
    tag_movies_iter(&tag->movies, &iter);
    assert(tag_movies_next(&iter, &movieid));
    assert(movieid == 92);

    tag = tags_search(&table, "good");
    assert(tag != NULL);
    assert(strcmp(tag->name, "good") == 0);
    assert(tag->movies.length == 3);

    tag_movies_iter(&tag->movies, &iter);
    while (tag_movies_next(&iter, &movieid)) {
        switch (movieid) {
            case 88:
                assert(!found88);
                found88 = true;
                break;
            case 90:
                assert(!found90);
                found90 = true;
                break;
            case 92:
                assert(!found92);
                found92 = true;
                break;
            default:
                assert(false);
                break;
        }
    }

    assert(found88 && found90 && found92);

    tags_destroy(&table);
    error_destroy(&error);

    puts("Ok");

    return 0;
}
