#include "movies.h"
#include "alloc.h"
#include "prime.h"

#define MAX_LOAD 0.5

#define GREEN "\e[92m"
#define YELLOW "\e[93m"
#define BLUE "\e[93m"
#define RED "\e[91m"
#define MAGENTA "\e[95m"
#define CLEAR "\e[0m"

/**
 * Converts a hash to an index in the table.
 */
static size_t hash_to_index(
        size_t capacity,
        uint_fast64_t hash,
        uint_fast64_t attempt);

/**
 * Resizes the table to have at least double capacity.
 */
static void resize(struct movies_table *restrict table, struct error *restrict error);

void movie_print(struct movie const *restrict movie)
{
    printf(MAGENTA "%llu"
            CLEAR ", "
            GREEN "%s"
            CLEAR ", "
            YELLOW "%s"
            CLEAR "\n",
            (long long unsigned) movie->id,
            movie->title,
            movie->genres);
}

void movies_init(
        struct movies_table *restrict table,
        size_t initial_capacity,
        struct error *restrict error)
{
    size_t i;

    table->length = 0;
    table->capacity = next_prime(initial_capacity);
    table->entries = moviedb_alloc(
            sizeof(struct movie **) * table->capacity,
            error);

    if (error->code == error_none) {
        for (i = 0; i < table->capacity; i++) {
            table->entries[i] = NULL;
        }
    }
}

void movies_insert(
        struct movies_table *restrict table,
        struct movie_csv_row *restrict movie_row,
        struct error *restrict error)
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
    index = hash_to_index(table->capacity, hash, attempt);

    while (table->entries[index] != NULL && error->code == error_none) {
        if (table->entries[index]->id == movie_row->id) {
            error_set_code(error, error_dup_movie_id);
            error->data.dup_movie_id.id = movie_row->id;
        } else {
            attempt++;
            index = hash_to_index(table->capacity, hash, attempt);
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
    size_t index = hash_to_index(table->capacity, hash, attempt);
    struct movie *movie = table->entries[index];

    while (movie != NULL && movie->id != movieid) {
        attempt++;
        index = hash_to_index(table->capacity, hash, attempt);
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
        size_t capacity,
        uint_fast64_t hash,
        uint_fast64_t attempt)
{
    uint_fast64_t term0 = hash % capacity;
    uint_fast64_t term1 = attempt % capacity;
    uint_fast64_t term2 = (term1 * term1) % capacity;
    
    return ((term0 + term1) % capacity + term2) % capacity;
}

static void resize(
        struct movies_table *restrict table,
        struct error *restrict error)
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
        for (i = 0; i < new_capacity; i++) {
            new_entries[i] = NULL;
        }

        for (i = 0; i < table->capacity; i++) {
            if (table->entries[i] != NULL) {
                hash = moviedb_id_hash(table->entries[i]->id);
                attempt = 0;
                index = hash_to_index(new_capacity, hash, attempt);

                while (new_entries[index] != NULL) {
                    attempt++;
                    index = hash_to_index(new_capacity, hash, attempt);
                }

                new_entries[index] = table->entries[i];
            }
        }

        moviedb_free(table->entries);
        table->entries = new_entries;
        table->capacity = new_capacity;
    }
}
