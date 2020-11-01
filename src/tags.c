#include "tags.h"
#include "prime.h"
#include "alloc.h"

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
        moviedb_id movie,
        struct error *restrict error);

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
    moviedb_hash hash;
    size_t index;

    load = (table->length + 1) / (double) table->capacity;
    if (load >= MAX_LOAD) {
        resize(table, error);
    }

    if (error->code == error_none) {
        hash = moviedb_hash_str(tag_row->name);
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
        }
    }
}

struct tag const *tags_search(
        struct tags_table const *restrict table,
        moviedb_id tagid)
{
    moviedb_hash hash = tag_name_hash(tagid);
    size_t index = probe_index(table, tagid, hash);
    return table->entries[index];
}

void tags_destroy(struct tags_table *restrict table)
{
    size_t i;

    for (i = 0; i < table->capacity; i++) {
        if (table->entries[i] != NULL) {
            moviedb_free(table->entries[i]->movies.entries);
            moviedb_free(table->entries[i]);
        }
    }

    moviedb_free(table->entries);
}

static struct tag *tag_init(
        struct tag_csv_row *restrict tag_row,
        struct error *restrict error)
{
    struct tag *tag = moviedb_alloc(sizeof(struct tag), error);

    if (error->code == error_none) {
        tag->name = tag_row->name;
        tag->movies.length = 1;
        tag->movies.capacity = 1;
        tag->movies.entries = moviedb_alloc(
                sizeof(struct tag_movie) * tag->movies.capacity,
                error);

        if (error->code == error_none) {
            tag->movies.entries[0] = tag_row->movieid;
        } else {
            moviedb_free(tag);
            tag = NULL;
        }
    }

    return tag;
}

static void tag_movies_insert(
        struct tag_movie_list *restrict movies,
        moviedb_id movie,
        struct error *restrict error)
{
    struct tag_movie *new_entries;
    size_t new_cap = movies->capacity * 2;

    if (movies->length == movies->capacity) {
        new_entries = moviedb_realloc(
                movies->entries,
                sizeof(struct moviedb_id) * new_cap,
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
        moviedb_id tagid,
        moviedb_hash hash)
{
    moviedb_hash attempt;
    size_t index;
    struct tag *tag;

    attempt = 0;
    index = moviedb_hash_to_index(hash, attempt, table->capacity);
    tag = table->entries[index];

    while (tag != NULL && tag->id != tagid) {
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
    moviedb_hash hash;
    size_t index;
    struct tags_table new_table;

    if (SIZE_MAX / 2 >= table->capacity) {
        new_table.capacity = next_prime(table->capacity * 2);
    } else {
        new_table.capacity = SIZE_MAX;
    }

    new_table.entries = moviedb_alloc(
            sizeof(struct tag **) * new_table.capacity,
            error);

    if (error->code == error_none) {
        for (i = 0; i < new_table.capacity; i++) {
            new_table.entries[i] = NULL;
        }

        for (i = 0; i < table->capacity; i++) {
            if (table->entries[i] != NULL) {
                hash = tag_name_hash(table->entries[i]->id);
                index = probe_index(&new_table, table->entries[i]->id, hash);
                new_table.entries[index] = table->entries[i];
            }
        }

        moviedb_free(table->entries);
        table->capacity = new_table.capacity;
        table->entries = new_table.entries;
    }
}
