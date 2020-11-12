#include <ctype.h>
#include "movies.h"
#include "alloc.h"
#include "prime.h"

#define MAX_LOAD 0.5

/**
 * Probes the given table until the place where the given movie ID should be
 * stored, given its hash. Returns the index of this place.
 */
static size_t probe_index(
        struct movies_table const *restrict table,
        moviedb_id_t movieid,
        moviedb_hash_t hash);

/**
 * Resizes the table to have at least double capacity.
 */
static void resize(
        struct movies_table *restrict table,
        struct error *restrict error);

bool movie_has_genre(
        struct movie const *restrict movie,
        char const *restrict genre)
{
    bool has_genre = false;
    /* Iterates over the movie's genre list. */
    size_t i = 0;
    /* Iterates over the input genre. */
    size_t j;
    bool searching;
    char const *restrict list = movie->genres;

    /*
     * Loops until it is confirmed that this movie has the given genre, or until
     * that all movie's genres have been tested.
     */
    while (!has_genre && list[i] != 0) {
        searching = true;
        j = 0;

        /*
         * Loops until either the end of one string is found or until characters
         * don't match.
         */
        while (!has_genre && searching) {
            /* End of the current genre in list AND end of input genre? */
            has_genre = (list[i] == 0 || list[i] == '|') && genre[j] == 0;

            /* NOT et the end of current genre AND NOT end of input genre? */
            searching = list[i] != 0 && list[i] != '|' && genre[j] != 0;
            /* ALSO, are characters at both strings equals? */
            searching = searching && list[i] == genre[j];

            /* Only advance if there is a next iteration again. */
            if (!has_genre && searching) {
                i++;
                j++;
            }
        }

        if (!has_genre) {
            /*
             * Skips the remaining characters in the current genre of the genre
             * list, so we test the next genre in the next attempt.
             */
            while (list[i] != 0 && list[i] != '|') {
                i++;
            }

            /*
             * Also skips the separator if the end of the string has not been
             * reached.
             */
            if (list[i] == '|') {
                i++;
            }
        }
    }

    return has_genre;
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

void movies_insert(
        struct movies_table *restrict table,
        struct movie_csv_row *restrict movie_row,
        struct error *restrict error)
{
    double load;
    moviedb_hash_t hash;
    size_t index;
    struct movie *movie = NULL;

    load = (table->length + 1) / (double) table->capacity;

    if (load >= MAX_LOAD) {
        /* Resize if it would be above maximum load. */
        resize(table, error);
    }

    if (error->code == error_none) {
        hash = moviedb_id_hash(movie_row->id);
        index = probe_index(table, movie_row->id, hash);
        if (table->entries[index] == NULL) {
            /* Allocates a movie to be inserted. */
            movie = moviedb_alloc(sizeof(*movie), 1, error);
        } else {
            /* Duplicated movie ID error. */
            error_set_code(error, error_dup_movie_id);
            error->data.dup_movie_id.id = movie_row->id;
        }

        if (error->code == error_none) {
            /* Finally inserts the movie. */
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
        moviedb_id_t movieid,
        double rating)
{
    unsigned long ratings;
    double sum;
    moviedb_hash_t hash = moviedb_id_hash(movieid);
    /* Index of the movie. */
    size_t index = probe_index(table, movieid, hash);

    if (table->entries[index] != NULL) {
        /* The given movie is present. */
        ratings = table->entries[index]->ratings;
        /* Sum of ratings. */
        sum = table->entries[index]->mean_rating * ratings + rating;
        /* Increase previous number of ratings. */
        ratings++;
        /* Registers new mean and saves current number of ratings. */
        table->entries[index]->mean_rating = sum / ratings;
        table->entries[index]->ratings = ratings;
    }
}

struct movie const *movies_search(
        struct movies_table const *restrict table,
        moviedb_id_t movieid)
{
    moviedb_hash_t hash = moviedb_id_hash(movieid);
    size_t index = probe_index(table, movieid, hash);
    return table->entries[index];
}

extern inline void movies_iter(
        struct movies_table const *table,
        struct movies_iter *restrict iter_out);

struct movie const *movies_next(struct movies_iter *restrict iter)
{
    struct movie const *movie = NULL;

    /*
     * Moves the iterator to the next position while entries are NULL and there
     * are entries left.
     */
    while (iter->current < iter->table->length && movie == NULL) {
        movie = iter->table->entries[iter->current];
        iter->current++;
    }

    return movie;
}

void movies_destroy(struct movies_table *restrict table)
{
    size_t i;

    /* Iterates through all entries to free their movie's memories. */
    for (i = 0; i < table->capacity; i++) {
        if (table->entries[i] != NULL) {
            moviedb_free((void *) (void const *) table->entries[i]->title);
            moviedb_free((void *) (void const *) table->entries[i]->genres);
            moviedb_free(table->entries[i]);
        }
    }

    moviedb_free(table->entries);
}

static size_t probe_index(
        struct movies_table const *restrict table,
        moviedb_id_t movieid,
        moviedb_hash_t hash)
{
    moviedb_hash_t attempt;
    size_t index;
    struct movie *movie;

    attempt = 0;
    index = moviedb_hash_to_index(hash, attempt, table->capacity);
    movie = table->entries[index];

    /* Iterates while there is a movie and it is not our target. */
    while (movie != NULL && movie->id != movieid) {
        /*
         * If we reached here, the condition failed, and we need to get the
         * next attempt.
         */
        attempt++;
        index = moviedb_hash_to_index(hash, attempt, table->capacity);
        movie = table->entries[index];
    }

    return index;
}

static void resize(
        struct movies_table *restrict table,
        struct error *restrict error)
{
    size_t i;
    moviedb_hash_t hash;
    size_t index;
    struct movies_table new_table;

    /*
     * Checks if there is a next prime with at least double capacity, in first
     * place.
     */
    if (SIZE_MAX / 2 < table->capacity) {
        new_table.capacity = SIZE_MAX;
    } else {
        new_table.capacity = next_prime(table->capacity * 2);
    }

    /* Sets an error if no prime available. */
    if (new_table.capacity == SIZE_MAX) {
        error_set_code(error, error_max_capacity);
        error->data.max_capacity.capacity = table->capacity;
    }

    if (error->code == error_none) {
        new_table.entries = moviedb_alloc(
                sizeof(*new_table.entries),
                new_table.capacity,
                error);
    }

    if (error->code == error_none) {
        /* Initializes the new table's entries. */
        for (i = 0; i < new_table.capacity; i++) {
            new_table.entries[i] = NULL;
        }

        /* Reinserts entries from old table into the new table. */
        for (i = 0; i < table->capacity; i++) {
            if (table->entries[i] != NULL) {
                hash = moviedb_id_hash(table->entries[i]->id);
                index = probe_index(&new_table, table->entries[i]->id, hash);
                new_table.entries[index] = table->entries[i];
            }
        }

        /* Frees the old table. */
        moviedb_free(table->entries);
        table->capacity = new_table.capacity;
        table->entries = new_table.entries;
    }
}
