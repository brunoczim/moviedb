#ifndef MOVIEDB_MOVIES_H
#define MOVIEDB_MOVIES_H 1

#include "error.h"
#include "id.h"
#include "csv/movie.h"

/**
 * This file exports items related to movie storage. More specifically, it
 * provides a hash table whose keys are movie IDs and data is movie data.
 */

/**
 * A movie's data.
 */
struct movie {
    /**
     * ID of the movie. Only internal movies hash table code is allowed to
     * update this, reading is fine.
     */
    db_id_t id;
    /**
     * Title of the movie. Heap-allocated. Only internal movies hash table code
     * is allowed to update this, reading is fine.
     */
    char const *title;
    /**
     * Genres of the movie. Heap-allocated.  Only internal movies hash table
     * code is allowed to update this, reading is fine.
     */
    char const *genres;
    /**
     * How many ratings were done on this movie.
     */
    unsigned long ratings;
    /**
     * Mean of the ratings.
     */
    double mean_rating;
};

/**
 * A hash table mapping movies' IDs to movies' data.
 */
struct movies_table {
    /**
     * Array of pointers to movie data. The entries of the table. Only internal
     * movie hash table code is allowed to touch this.
     */
    struct movie **entries;
    /**
     * How many movies are stored in this hash table. The entries of the table.
     * Only internal movie hash table code is allowed to write to this. Reading
     * is fine.
     */
    size_t length;
    /**
     * How many movies can be stored in this hash table. The entries of the
     * table. Only internal movie hash table code is allowed to write to this.
     * Reading is fine.
     */
    size_t capacity;
};

void movie_print(struct movie const *restrict movie);

/**
 * Initializes the hash table. Initial capacity is rounded to the smallest prime
 * such that actual_initial_capacity >= initial_capacity.
 */
void movies_init(
        struct movies_table *restrict table,
        size_t initial_capacity,
        struct error *restrict error);

/**
 * Inserts a movie CSV row in the table. Title and genres of the CSV row should
 * be heap-allocated, since the table will free them. If movie ID is duplicated,
 * an error is set. The given movie_row should not be used after this function
 * is called.
 */
void movies_insert(
        struct movies_table *restrict table,
        struct movie_csv_row *restrict movie_row,
        struct error *restrict error);

/**
 * Adds a rating for a movie. If not found, rating is not added.
 */
void movies_add_rating(
        struct movies_table *restrict table,
        db_id_t movieid,
        double rating);

/**
 * Search for the movie with the given ID. Returns NULL if not found.
 */
struct movie const *movies_search(
        struct movies_table const *restrict table,
        db_id_t movieid);

/**
 * Destroys everything in the table. Movies. Titles. Genres. Entries.
 * Everything.
 */
void movies_destroy(struct movies_table *restrict table);

#endif
