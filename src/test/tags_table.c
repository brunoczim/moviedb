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
    assert(tag->movies.entries[0] == 92);

    tag = tags_search(&table, "good");
    assert(tag != NULL);
    assert(strcmp(tag->name, "good") == 0);
    assert(tag->movies.length == 3);
    assert(tag->movies.entries[0] == 90);
    assert(tag->movies.entries[1] == 92);
    assert(tag->movies.entries[2] == 88);

    insert(&table, "good", 96, &error);
    assert(error.code == error_none);
    insert(&table, "good", 89, &error);
    assert(error.code == error_none);
    insert(&table, "good", 83, &error);
    assert(error.code == error_none);
    insert(&table, "good", 158, &error);
    assert(error.code == error_none);
    insert(&table, "good", 87, &error);
    assert(error.code == error_none);
    insert(&table, "good", 0, &error);
    assert(error.code == error_none);
    insert(&table, "good", 957, &error);
    assert(error.code == error_none);
    assert(error.code == error_none);
    insert(&table, "good", 2, &error);
    assert(error.code == error_none);
    insert(&table, "good", 3, &error);
    assert(error.code == error_none);
    insert(&table, "good", 4, &error);
    assert(error.code == error_none);
    insert(&table, "good", 5, &error);
    assert(error.code == error_none);
    insert(&table, "good", 6, &error);
    assert(error.code == error_none);
    insert(&table, "good", 7, &error);
    assert(error.code == error_none);
    insert(&table, "good", 8, &error);
    assert(error.code == error_none);
    insert(&table, "good", 9, &error);
    assert(error.code == error_none);
    insert(&table, "good", 1, &error);
    assert(error.code == error_none);

    tag = tags_search(&table, "good");
    assert(tag != NULL);
    assert(strcmp(tag->name, "good") == 0);
    assert(tag->movies.length == 19);

    tags_sort_movies(&table, &error);
    assert(error.code == error_none);

    tag = tags_search(&table, "good");
    assert(tag != NULL);
    assert(strcmp(tag->name, "good") == 0);
    assert(tag->movies.length == 19);
    assert(tag->movies.entries[0] == 0);
    assert(tag->movies.entries[1] == 1);
    assert(tag->movies.entries[2] == 2);
    assert(tag->movies.entries[3] == 3);
    assert(tag->movies.entries[4] == 4);
    assert(tag->movies.entries[5] == 5);
    assert(tag->movies.entries[6] == 6);
    assert(tag->movies.entries[7] == 7);
    assert(tag->movies.entries[8] == 8);
    assert(tag->movies.entries[9] == 9);
    assert(tag->movies.entries[10] == 83);
    assert(tag->movies.entries[11] == 87);
    assert(tag->movies.entries[12] == 88);
    assert(tag->movies.entries[13] == 89);
    assert(tag->movies.entries[14] == 90);
    assert(tag->movies.entries[15] == 92);
    assert(tag->movies.entries[16] == 96);
    assert(tag->movies.entries[17] == 158);
    assert(tag->movies.entries[18] == 957);

    tags_destroy(&table);
    error_destroy(&error);

    puts("Ok");

    return 0;
}
