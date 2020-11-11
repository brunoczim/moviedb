#ifndef MOVIEDB_TAGS_MOVIES_H
#define MOVIEDB_TAGS_MOVIES_H 1

#include "../id.h"
#include "../alloc.h"

/**
 * This file provides utilites related to movie lists of tags.
 */

/**
 * The (hash) set of movies associated with a tag.
 */
struct tag_movie_set {
    /**
     * Array of entries. Only internal tag movies hash set code is allowed to
     * touch this value.
     */
    moviedb_id_t *entries;
    /**
     * Array of occupied flags, telling whether an entry of the same index is
     * occupied. Only internal tag movies hash set code is allowed to touch
     * this value.
     */
    bool *occupied;
    /**
     * How much entries are stored here. Only internal tag movies hash set
     * code is allowed to update this value. Reading is fine.
     */
    size_t length;
    /**
     * How many entries we can currently store. Only internal tag movies hash
     * set code is allowed to touch this value.
     */
    size_t capacity;
};

/**
 * Iterator over the tag_movies stored in a tag movies hash set.
 */
struct tag_movies_iter {
    /**
     * The set being iterated over. Only internal tag movies hash set code is
     * allowed to touch this.
     */
    struct tag_movie_set const *set;
    /**
     * The current entry being checked. Only internal tag movies hash set code
     * is allowed to touch this.
     */
    size_t current;
};

/**
 * Initializes the hash set. Initial capacity is rounded to the smallest prime
 * such that actual_initial_capacity >= initial_capacity.
 */
void tag_movies_init(
        struct tag_movie_set *restrict set,
        size_t initial_capacity,
        struct error *restrict error);

/**
 * Inserts a movie ID in the set. If movie ID is duplicated, an error is set
 * (error_dup_movie_id).
 */
void tag_movies_insert(
        struct tag_movie_set *restrict set,
        moviedb_id_t movieid,
        struct error *restrict error);

/**
 * Adds a rating for a movie. If not found, rating is not added.
 */
void tag_movies_add_rating(
        struct tag_movie_set *restrict set,
        moviedb_id_t movieid,
        double rating);

/**
 * Search for the movie with the given ID. Returns whether it was found.
 */
bool tag_movies_contain(
        struct tag_movie_set const *restrict set,
        moviedb_id_t movieid);

/**
 * Initializes an iterator over the given set.
 */
inline void tag_movies_iter(
        struct tag_movie_set const *set,
        struct tag_movies_iter *restrict iter_out)
{
    iter_out->set = set;
    iter_out->current = 0;
}

/**
 * Finds the next entry in the tag movies set, using the given iterator.
 * Returns whether there was an entry. If there was, it is placed in
 * movieid_out.
 */
bool tag_movies_next(
        struct tag_movies_iter *restrict iter,
        moviedb_id_t *restrict movieid_out);

/**
 * Destroys everything in the set.
 */
inline void tag_movies_destroy(struct tag_movie_set *restrict set)
{
    moviedb_free(set->entries);
    moviedb_free(set->occupied);
}

#endif
