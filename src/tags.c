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
 * Probes the given table until the place where the given tag name should be
 * stored, given its hash. Returns the index of this place.
 */
static size_t probe_index(
        struct tags_table const *restrict table,
        char const *restrict name,
        moviedb_hash_t hash);

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
    table->entries = moviedb_alloc(
            sizeof(*table->entries),
            table->capacity,
            error);

    if (error->code == error_none) {
        /* Initializes all entries to NULL. */
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
    moviedb_hash_t hash;
    size_t index;

    load = (table->length + 1) / (double) table->capacity;
    if (load >= MAX_LOAD) {
        /* Resize if it would be above maximum load. */
        resize(table, error);
    }

    if (error->code == error_none) {
        hash = moviedb_hash_str(tag_row->name);
        index = probe_index(table, tag_row->name, hash);

        if (table->entries[index] == NULL) {
            /* No previous insert with the given tag name. */
            table->entries[index] = tag_init(tag_row, error);
            if (error->code == error_none) {
                table->length++;
            }
        } else {
            /* There was a previous insert with the given tag name. */
            tag_movies_insert(
                    &table->entries[index]->movies,
                    tag_row->movieid,
                    error);

            /*
             * We can destroy the row's data, since we don't need the tag name,
             * which was heap-allocated.
             */
            tag_row_destroy(tag_row);
        }
    }
}

struct tag const *tags_search(
        struct tags_table const *restrict table,
        char const *restrict name)
{
    moviedb_hash_t hash = moviedb_hash_str(name);
    size_t index = probe_index(table, name, hash);
    return table->entries[index];
}

void tags_destroy(struct tags_table *restrict table)
{
    size_t i;

    /* Iterates through all entries to free their user's memories. */
    for (i = 0; i < table->capacity; i++) {
        if (table->entries[i] != NULL) {
            tag_movies_destroy(&table->entries[i]->movies);
            moviedb_free((void *) (void const *) table->entries[i]->name);
            moviedb_free(table->entries[i]);
        }
    }

    moviedb_free(table->entries);
}

static struct tag *tag_init(
        struct tag_csv_row *restrict tag_row,
        struct error *restrict error)
{
    struct tag *tag = moviedb_alloc(sizeof(*tag), 1, error);

    if (error->code == error_none) {
        /* Initializes the tag. */
        tag->name = tag_row->name;
        tag_movies_init(&tag->movies, 2, error);

        if (error->code == error_none) {
            /* Inserts the given movie ID into the tag's movie list. */
            tag_movies_insert(&tag->movies, tag_row->movieid, error);
        }

        if (error->code != error_none) {
            tag_movies_destroy(&tag->movies);
            moviedb_free(tag);
            tag = NULL;
        }
    }

    return tag;
}

static size_t probe_index(
        struct tags_table const *restrict table,
        char const *restrict name,
        moviedb_hash_t hash)
{
    moviedb_hash_t attempt;
    size_t index;
    struct tag *tag;

    attempt = 0;
    index = moviedb_hash_to_index(hash, attempt, table->capacity);
    tag = table->entries[index];

    /* Iterates while there is a tag and it is not our target. */
    while (tag != NULL && strcmp(tag->name, name) != 0) {
        /*
         * If we reached here, the condition failed, and we need to get the
         * next attempt.
         */
        attempt++;
        index = moviedb_hash_to_index(hash, attempt, table->capacity);
        tag = table->entries[index];
    }

    return index;
}

static void resize(
        struct tags_table *restrict table,
        struct error *restrict error)
{
    size_t i;
    moviedb_hash_t hash;
    size_t index;
    struct tags_table new_table;

    new_table.capacity = next_prime(table->capacity * 2);

    new_table.entries = moviedb_alloc(
            sizeof(*new_table.entries),
            new_table.capacity,
            error);

    if (error->code == error_none) {
        /* Initializes the new table's entries. */
        for (i = 0; i < new_table.capacity; i++) {
            new_table.entries[i] = NULL;
        }

        /* Reinserts entries from old table into the new table. */
        for (i = 0; i < table->capacity; i++) {
            if (table->entries[i] != NULL) {
                hash = moviedb_hash_str(table->entries[i]->name);
                index = probe_index(&new_table, table->entries[i]->name, hash);
                new_table.entries[index] = table->entries[i];
            }
        }

        /* Frees the old table. */
        moviedb_free(table->entries);
        table->capacity = new_table.capacity;
        table->entries = new_table.entries;
    }
}
