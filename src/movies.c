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
        struct movies_table const *restrict table,
        uint_fast64_t hash,
        uint_fast64_t attempt);

/**
 * Probes the given table until the place where the given movie ID should be
 * stored, given its hash. Returns the index of this place.
 */
static size_t probe_index(
        struct movies_table const *restrict table,
        moviedb_id movieid,
        uint_fast64_t hash);

/**
 * Resizes the table to have at least double capacity.
 */
static void resize(
        struct movies_table *restrict table,
        struct error *restrict error);

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
    size_t index;
    struct movie *movie;

    load = (table->length + 1) / (double) table->capacity;

    if (load >= MAX_LOAD) {
        resize(table, error);
    }

    if (error->code == error_none) {
        hash = moviedb_id_hash(movie_row->id);
        index = probe_index(table, movie_row->id, hash);
        if (table->entries[index] == NULL) {
            movie = moviedb_alloc(sizeof(struct movie), error);
        } else {
            error_set_code(error, error_dup_movie_id);
            error->data.dup_movie_id.id = movie_row->id;
        }

        if (error->code == error_none) {
            movie->id = movie_row->id;
            movie->title = movie_row->title;
            movie->genres = movie_row->genres;
            movie->ratings = 0;
            movie->mean_rating = 0.0;
            table->entries[index] = movie;
            table->length++;
        }
    }
}

void movies_add_rating(
        struct movies_table *restrict table,
        moviedb_id movieid,
        double rating)
{
    unsigned long ratings;
    double sum;
    uint_fast64_t hash = moviedb_id_hash(movieid);
    size_t index = probe_index(table, movieid, hash);

    if (table->entries[index] != NULL) {
        ratings = table->entries[index]->ratings;
        sum = table->entries[index]->mean_rating * ratings + rating;
        ratings++;
        table->entries[index]->mean_rating = sum / ratings;
        table->entries[index]->ratings = ratings;
    }
}

struct movie const *movies_search(
        struct movies_table const *restrict table,
        moviedb_id movieid)
{
    uint_fast64_t hash = moviedb_id_hash(movieid);
    size_t index = probe_index(table, movieid, hash);
    return table->entries[index];
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
    uint_fast64_t sum0 = (term0 + term1) % table->capacity;
    
    return (sum0 + term2) % table->capacity;
}

static size_t probe_index(
        struct movies_table const *restrict table,
        moviedb_id movieid,
        uint_fast64_t hash)
{
    uint_fast64_t attempt;
    size_t index;
    struct movie *movie;

    attempt = 0;
    index = hash_to_index(table, hash, attempt);
    movie = table->entries[index];

    while (movie != NULL && movie->id != movieid) {
        attempt++;
        index = hash_to_index(table, hash, attempt);
        movie = table->entries[index];
    }

    return index;
}

static void resize(
        struct movies_table *restrict table,
        struct error *restrict error)
{
    size_t i;
    uint_fast64_t hash;
    size_t index;
    struct movies_table new_table;

    if (SIZE_MAX / 2 >= table->capacity) {
        new_table.capacity = next_prime(table->capacity * 2);
    } else {
        new_table.capacity = SIZE_MAX;
    }

    new_table.entries = moviedb_alloc(
            sizeof(struct movie **) * new_table.capacity,
            error);

    if (error->code == error_none) {
        for (i = 0; i < new_table.capacity; i++) {
            new_table.entries[i] = NULL;
        }

        for (i = 0; i < table->capacity; i++) {
            if (table->entries[i] != NULL) {
                hash = moviedb_id_hash(table->entries[i]->id);
                index = probe_index(&new_table, table->entries[i]->id, hash);
                new_table.entries[index] = table->entries[i];
            }
        }

        moviedb_free(table->entries);
        table->capacity = new_table.capacity;
        table->entries = new_table.entries;
    }
}
