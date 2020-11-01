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
        db_id_t movie,
        struct error *restrict error)
{
    printf("Inserting %s\n", name);

    char *heap_name;
    struct tag_csv_row row;

    row.movieid = movie;

    heap_name = db_alloc(strlen(name) + 1, error);
    assert(error->code == error_none);
    strcpy(heap_name, name);
    row.name = heap_name;

    tags_insert(table, &row, error);

    if (error->code != error_none) {
        db_free(heap_name);
    }
}

int main(int argc, char const *argv[])
{
    struct error error;
    struct tag const *tag;
    struct tags_table table;

    error_init(&error);

    tags_init(&table, 5, &error);
    assert(error.code == error_none);
    assert(table.length == 0);
    assert(table.capacity == 5);

    insert(&table, "good", 123, &error);
    assert(error.code == error_none);
    assert(table.length == 1);
    assert(table.capacity == 5);

    tag = tags_search(&table, "good");
    assert(tag != NULL);
    assert(strcmp(tag->name, "good") == 0);

    insert(&table, "bad", 456, &error);
    assert(error.code == error_none);
    assert(table.length == 2);
    assert(table.capacity == 5);

    insert(&table, "average", 789, &error);
    assert(error.code == error_none);
    assert(table.length == 3);
    assert(table.capacity == 11);

    tag = tags_search(&table, "good");
    assert(tag != NULL);
    assert(strcmp(tag->name, "good") == 0);

    tag = tags_search(&table, "bad");
    assert(tag != NULL);
    assert(strcmp(tag->name, "bad") == 0);

    insert(&table, "good", 456, &error);
    assert(error.code == error_none);
    assert(table.length == 3);
    assert(table.capacity == 11);

    insert(&table, "good", 789, &error);
    assert(error.code == error_none);
    assert(table.length == 3);
    assert(table.capacity == 11);

    tag = tags_search(&table, "bad");
    assert(tag != NULL);
    assert(strcmp(tag->name, "bad") == 0);
    assert(tag->movies.length == 1);
    assert(tag->movies.entries[0] == 456);

    tag = tags_search(&table, "good");
    assert(tag != NULL);
    assert(strcmp(tag->name, "good") == 0);
    assert(tag->movies.length == 3);
    assert(tag->movies.entries[0] == 123);
    assert(tag->movies.entries[1] == 456);
    assert(tag->movies.entries[2] == 789);

    tags_destroy(&table);
    error_destroy(&error);

    puts("Ok");

    return 0;
}