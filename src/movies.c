#include "movies.h"
#include "alloc.h"
#include "prime.h"

#define MAX_LOAD 0.5

#define GREEN "\e[92m"
#define YELLOW "\e[93m"
#define BLUE "\e[94m"
#define RED "\e[91m"
#define MAGENTA "\e[95m"
#define CLEAR "\e[0m"

/**
 * Probes the given table until the place where the given movie ID should be
 * stored, given its hash. Returns the index of this place.
 */
static size_t probe_index(
        struct movies_table const *restrict table,
        db_id_t movieid,
        db_hash_t hash);

/**
 * Resizes the table to have at least double capacity.
 */
static void resize(
        struct movies_table *restrict table,
        struct error *restrict error);

void movie_print(struct movie const *restrict movie)
{
    char id_buffer[MOVIEDB_ID_DIGITS + 1];
    size_t id_start;

    id_start = db_id_to_str(movie->id, id_buffer, MOVIEDB_ID_DIGITS + 1);

    printf(MAGENTA "%s"
            CLEAR ", "
            GREEN "%s"
            CLEAR ", "
            YELLOW "%s"
            CLEAR ", "
            RED "%.1lf"
            CLEAR ", "
            BLUE "%zu"
            CLEAR "\n",
            id_buffer + id_start,
            movie->title,
            movie->genres,
            movie->mean_rating,
            movie->ratings);
}

void movies_init(
        struct movies_table *restrict table,
        size_t initial_capacity,
        struct error *restrict error)
{
    size_t i;

    table->length = 0;
    table->capacity = next_prime(initial_capacity);
    table->entries = db_alloc(
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
    db_hash_t hash;
    size_t index;
    struct movie *movie;

    load = (table->length + 1) / (double) table->capacity;

    if (load >= MAX_LOAD) {
        resize(table, error);
    }

    if (error->code == error_none) {
        hash = db_id_hash(movie_row->id);
        index = probe_index(table, movie_row->id, hash);
        if (table->entries[index] == NULL) {
            movie = db_alloc(sizeof(struct movie), error);
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
        db_id_t movieid,
        double rating)
{
    unsigned long ratings;
    double sum;
    db_hash_t hash = db_id_hash(movieid);
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
        db_id_t movieid)
{
    db_hash_t hash = db_id_hash(movieid);
    size_t index = probe_index(table, movieid, hash);
    return table->entries[index];
}

void movies_destroy(struct movies_table *restrict table)
{
    size_t i;

    for (i = 0; i < table->capacity; i++) {
        if (table->entries[i] != NULL) {
            db_free((void *) (void const *) table->entries[i]->title);
            db_free((void *) (void const *) table->entries[i]->genres);
            db_free(table->entries[i]);
        }
    }

    db_free(table->entries);
}

static size_t probe_index(
        struct movies_table const *restrict table,
        db_id_t movieid,
        db_hash_t hash)
{
    db_hash_t attempt;
    size_t index;
    struct movie *movie;

    attempt = 0;
    index = db_hash_to_index(hash, attempt, table->capacity);
    movie = table->entries[index];

    while (movie != NULL && movie->id != movieid) {
        attempt++;
        index = db_hash_to_index(hash, attempt, table->capacity);
        movie = table->entries[index];
    }

    return index;
}

static void resize(
        struct movies_table *restrict table,
        struct error *restrict error)
{
    size_t i;
    db_hash_t hash;
    size_t index;
    struct movies_table new_table;

    if (SIZE_MAX / 2 >= table->capacity) {
        new_table.capacity = next_prime(table->capacity * 2);
    } else {
        new_table.capacity = SIZE_MAX;
    }

    new_table.entries = db_alloc(
            sizeof(struct movie **) * new_table.capacity,
            error);

    if (error->code == error_none) {
        for (i = 0; i < new_table.capacity; i++) {
            new_table.entries[i] = NULL;
        }

        for (i = 0; i < table->capacity; i++) {
            if (table->entries[i] != NULL) {
                hash = db_id_hash(table->entries[i]->id);
                index = probe_index(&new_table, table->entries[i]->id, hash);
                new_table.entries[index] = table->entries[i];
            }
        }

        db_free(table->entries);
        table->capacity = new_table.capacity;
        table->entries = new_table.entries;
    }
}
