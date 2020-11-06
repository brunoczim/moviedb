#include "tags.h"
#include "prime.h"
#include "alloc.h"
#include <string.h>

#define MAX_LOAD 0.5

/**
 * Initializes a tag by allocating a tag in the heap.
 */
static struct tag *tag_init(
        struct tag_csv_row *restrict tag_row,
        struct error *restrict error);

/**
 * Inserts a movie in the given tag movie list.
 */
static void tag_movies_insert(
        struct tag_movie_list *restrict movies,
        db_id_t movie,
        struct error *restrict error);

/**
 * Probes the given table until the place where the given tag name should be
 * stored, given its hash. Returns the index of this place.
 */
static size_t probe_index(
        struct tags_table const *restrict table,
        char const *restrict name,
        db_hash_t hash);

/**
 * Resizes the table to have at least double capacity.
 */
static void resize(
        struct tags_table *restrict table,
        struct error *restrict error);

void tags_init(
        struct tags_table *restrict table,
        size_t initial_capacity,
        struct error *restrict error)
{
    size_t i;

    table->length = 0;
    table->capacity = next_prime(initial_capacity);
    table->entries = db_alloc(
            sizeof(struct tag **) * table->capacity,
            error);

    if (error->code == error_none) {
        for (i = 0; i < table->capacity; i++) {
            table->entries[i] = NULL;
        }
    }
}

void tags_insert(
        struct tags_table *restrict table,
        struct tag_csv_row *restrict tag_row,
        struct error *restrict error)
{
    double load;
    db_hash_t hash;
    size_t index;

    load = (table->length + 1) / (double) table->capacity;
    if (load >= MAX_LOAD) {
        resize(table, error);
    }

    if (error->code == error_none) {
        hash = db_hash_str(tag_row->name);
        index = probe_index(table, tag_row->name, hash);

        if (table->entries[index] == NULL) {
            table->entries[index] = tag_init(tag_row, error);
            if (error->code == error_none) {
                table->length++;
            }
        } else {
            tag_movies_insert(
                    &table->entries[index]->movies,
                    tag_row->movieid,
                    error);

            tag_row_destroy(tag_row);
        }
    }
}

struct tag const *tags_search(
        struct tags_table const *restrict table,
        char const *restrict name)
{
    db_hash_t hash = db_hash_str(name);
    size_t index = probe_index(table, name, hash);
    return table->entries[index];
}

void tags_sort_movies(
        struct tags_table *restrict table,
        struct error *restrict error)
{
    struct tag_movies_sort_stack stack;
    size_t i = 0;

    tag_movies_sort_init(&stack, 2, error);

    if (error->code == error_none) {
        while (i < table->capacity && error->code == error_none) {
            if (table->entries[i] != NULL) {
                tag_movies_sort(&table->entries[i]->movies, &stack, error);
            }
            i++;
        }
        tag_movies_sort_destroy(&stack);
    }
}

void tags_destroy(struct tags_table *restrict table)
{
    size_t i;

    for (i = 0; i < table->capacity; i++) {
        if (table->entries[i] != NULL) {
            db_free(table->entries[i]->movies.entries);
            db_free((void *) (void const *) table->entries[i]->name);
            db_free(table->entries[i]);
        }
    }

    db_free(table->entries);
}

static struct tag *tag_init(
        struct tag_csv_row *restrict tag_row,
        struct error *restrict error)
{
    struct tag *tag = db_alloc(sizeof(struct tag), error);

    if (error->code == error_none) {
        tag->name = tag_row->name;
        tag->movies.length = 1;
        tag->movies.capacity = 1;
        tag->movies.entries = db_alloc(
                sizeof(struct tag) * tag->movies.capacity,
                error);

        if (error->code == error_none) {
            tag->movies.entries[0] = tag_row->movieid;
        } else {
            db_free(tag);
            tag = NULL;
        }
    }

    return tag;
}

static void tag_movies_insert(
        struct tag_movie_list *restrict movies,
        db_id_t movie,
        struct error *restrict error)
{
    db_id_t *new_entries;
    size_t new_cap = movies->capacity * 2;

    if (movies->length == movies->capacity) {
        new_entries = db_realloc(
                movies->entries,
                sizeof(db_id_t) * new_cap,
                error);

        if (error->code == error_none) {
            movies->entries = new_entries;
            movies->capacity = new_cap;
        }
    }

    if (error->code == error_none) {
        movies->entries[movies->length] = movie;
        movies->length++;
    }
}

static size_t probe_index(
        struct tags_table const *restrict table,
        char const *restrict name,
        db_hash_t hash)
{
    db_hash_t attempt;
    size_t index;
    struct tag *tag;

    attempt = 0;
    index = db_hash_to_index(hash, attempt, table->capacity);
    tag = table->entries[index];

    while (tag != NULL && strcmp(tag->name, name) != 0) {
        attempt++;
        index = db_hash_to_index(hash, attempt, table->capacity);
        tag = table->entries[index];
    }

    return index;
}

static void resize(
        struct tags_table *restrict table,
        struct error *restrict error)
{
    size_t i;
    db_hash_t hash;
    size_t index;
    struct tags_table new_table;

    if (SIZE_MAX / 2 >= table->capacity) {
        new_table.capacity = next_prime(table->capacity * 2);
    } else {
        new_table.capacity = SIZE_MAX;
    }

    new_table.entries = db_alloc(
            sizeof(struct tag **) * new_table.capacity,
            error);

    if (error->code == error_none) {
        for (i = 0; i < new_table.capacity; i++) {
            new_table.entries[i] = NULL;
        }

        for (i = 0; i < table->capacity; i++) {
            if (table->entries[i] != NULL) {
                hash = db_hash_str(table->entries[i]->name);
                index = probe_index(&new_table, table->entries[i]->name, hash);
                new_table.entries[index] = table->entries[i];
            }
        }

        db_free(table->entries);
        table->capacity = new_table.capacity;
        table->entries = new_table.entries;
    }
}
