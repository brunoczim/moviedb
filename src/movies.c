#include "movies.h"
#include "alloc.h"
#include "prime.h"

#define MAX_LOAD 0.5

static size_t hash_to_index(
        struct movies_table const *restrict table,
        uint_fast64_t hash,
        uint_fast64_t attempt);

static void resize(struct movies_table *restrict table, struct error *error);

void movies_init(
        struct movies_table *restrict table,
        size_t initial_capacity,
        struct error *error)
{
    table->length = 0;
    table->capacity = next_prime(initial_capacity);
    table->entries = moviedb_alloc(
            sizeof(struct movie **) * table->capacity,
            error);
}

void movies_insert(
        struct movies_table *restrict table,
        struct movie_csv_row *restrict movie_row,
        struct error *error)
{
    double load;
    uint_fast64_t hash;
    uint_fast64_t attempt;
    size_t index;
    struct movie *movie;

    load = (table->length + 1) / (double) table->capacity;

    if (load >= MAX_LOAD) {
        resize(table, error);
    }

    hash = moviedb_id_hash(movie_row->id);
    attempt = 0;
    index = hash_to_index(table, hash, attempt);

    while (table->entries[index] != NULL && error->code == error_none) {
        if (table->entries[index]->id == movie_row->id) {
            error_set_code(error, error_dup_movie_id);
            error->data.dup_movie_id.id = movie_row->id;
        } else {
            attempt++;
            index = hash_to_index(table, hash, attempt);
        }
    }

    if (error->code == error_none) {
        movie = moviedb_alloc(sizeof(struct movie), error);
        if (error->code == error_none) {
            movie->id = movie_row->id;
            movie->title = movie_row->title;
            movie->genres = movie_row->genres;
            table->entries[index] = movie;
            table->length++;
        }
    }
}

struct movie const *movies_search(
        struct movies_table const *restrict table,
        moviedb_id movieid)
{
    uint_fast64_t hash = moviedb_id_hash(movieid);
    uint_fast64_t attempt = 0;
    size_t index = hash_to_index(table, hash, attempt);
    struct movie *movie = table->entries[index];

    while (movie != NULL && movie->id != movieid) {
        attempt++;
        index = hash_to_index(table, hash, attempt);
        movie = table->entries[index];
    }

    return movie;
}

void movies_destroy(struct movies_table *restrict table)
{
    size_t i;

    for (i = 0; i < table->capacity; i++) {
        if (table->entries[i] != NULL) {
            moviedb_free((void *) (void const *) table->entries[i]->title);
            moviedb_free((void *) (void const *) table->entries[i]->genres);
            moviedb_free(table->entries[i]);
        }
    }

    moviedb_free(table->entries);
}

static size_t hash_to_index(
        struct movies_table const *restrict table,
        uint_fast64_t hash,
        uint_fast64_t attempt)
{
    uint_fast64_t term0 = hash % table->capacity;
    uint_fast64_t term1 = attempt % table->capacity;
    uint_fast64_t term2 = (term1 * term1) % table->capacity;
    
    return ((term0 + term1) % table->capacity + term2) % table->capacity;
}

static void resize(
        struct movies_table *restrict table,
        struct error *error)
{
    size_t new_capacity;
    size_t i;
    uint_fast64_t hash;
    uint_fast64_t attempt;
    size_t index;
    struct movie **new_entries;

    if (SIZE_MAX / 2 >= table->capacity) {
        new_capacity = next_prime(table->capacity * 2);
    } else {
        new_capacity = SIZE_MAX;
    }

    new_entries = moviedb_alloc(sizeof(struct movie **) * new_capacity, error);

    if (error->code == error_none) {
        for (i = 0; i < table->capacity; i++) {
            if (table->entries[i] != NULL) {
                hash = moviedb_id_hash(table->entries[i]->id);
                attempt = 0;
                index = hash_to_index(table, hash, attempt);

                while (table->entries[index] != NULL) {
                    attempt++;
                    index = hash_to_index(table, hash, attempt);
                }

                new_entries[index] = table->entries[i];
            }
        }

        moviedb_free(table->entries);
        table->entries = new_entries;
        table->capacity = new_capacity;
    }
}
