#include "movies.h"
#include "../prime.h"

#define MAX_LOAD 0.5

/**
 * Probes the given hash set until the place where the given movie ID should be
 * stored, given its hash. Returns the index of this place.
 */
static size_t probe_index(
        struct tag_movie_set const *restrict set,
        moviedb_id_t movieid,
        moviedb_hash_t hash);

/**
 * Resizes the hash set to have at least double capacity.
 */
static void resize(
        struct tag_movie_set *restrict set,
        struct error *restrict error);

void tag_movies_init(
        struct tag_movie_set *restrict set,
        size_t initial_capacity,
        struct error *restrict error)
{
    size_t i;

    set->length = 0;
    set->capacity = next_prime(initial_capacity);
    set->entries = moviedb_alloc(sizeof(moviedb_id_t) * set->capacity, error);

    if (error->code == error_none) {
        set->occupied = moviedb_alloc(sizeof(bool) * set->capacity, error);

        if (error->code != error_none) {
            moviedb_free(set->entries);
            set->entries = NULL;
        }
    }

    if (error->code == error_none) {
        /* Initializes all occupied flags to false. */
        for (i = 0; i < set->capacity; i++) {
            set->occupied[i] = false;
        }
    }
}

void tag_movies_insert(
        struct tag_movie_set *restrict set,
        moviedb_id_t movieid,
        struct error *restrict error)
{
    double load;
    moviedb_hash_t hash;
    size_t index;

    load = (set->length + 1) / (double) set->capacity;

    if (load >= MAX_LOAD) {
        /* Resize if it would be above maximum load. */
        resize(set, error);
    }

    if (error->code == error_none) {
        hash = moviedb_id_hash(movieid);
        index = probe_index(set, movieid, hash);
        if (set->occupied[index]) {
            /* Duplicated movie ID is an error, but ignorable. */
            error_set_code(error, error_dup_movie_id);
            error->data.dup_movie_id.id = movieid;
        } else {
            /* Success case for the insert. */
            set->occupied[index] = true;
            set->entries[index] = movieid;
            set->length++;
        }
    }
}

bool tag_movies_contain(
        struct tag_movie_set const *restrict set,
        moviedb_id_t movieid)
{
    moviedb_hash_t hash = moviedb_id_hash(movieid);
    size_t index = probe_index(set, movieid, hash);
    return set->occupied[index];
}

extern inline void tag_movies_iter(
        struct tag_movie_set const *set,
        struct tag_movies_iter *restrict iter_out);

bool tag_movies_next(
        struct tag_movies_iter *restrict iter,
        moviedb_id_t *restrict movieid_out)
{
    bool found = false;

    /*
     * Moves the iterator to the next position while entries are not occupied
     * and there are entries left.
     */
    while (iter->current < iter->set->capacity && !found) {
        found = iter->set->occupied[iter->current];
        if (found) {
            *movieid_out = iter->set->entries[iter->current];
        }
        iter->current++;
    }

    return found;
}

extern inline void tag_movies_destroy(struct tag_movie_set *restrict set);

static size_t probe_index(
        struct tag_movie_set const *restrict set,
        moviedb_id_t movieid,
        moviedb_hash_t hash)
{
    moviedb_hash_t attempt;
    size_t index;

    attempt = 0;
    index = moviedb_hash_to_index(hash, attempt, set->capacity);

    /* Iterates while entry is occupied and it is not our target. */
    while (set->occupied[index] && set->entries[index] != movieid) {
        /*
         * If we reached here, the condition failed, and we need to get the
         * next attempt.
         */
        attempt++;
        index = moviedb_hash_to_index(hash, attempt, set->capacity);
    }

    return index;
}

static void resize(
        struct tag_movie_set *restrict set,
        struct error *restrict error)
{
    size_t i;
    moviedb_hash_t hash;
    size_t index;
    struct tag_movie_set new_set;

    new_set.capacity = next_prime(set->capacity * 2);

    new_set.entries = moviedb_alloc(
            sizeof(moviedb_id_t) * new_set.capacity,
            error);

    if (error->code == error_none) {
        new_set.occupied = moviedb_alloc(
                sizeof(bool) * new_set.capacity,
                error);

        if (error->code != error_none) {
            moviedb_free(new_set.entries);
        }
    }

    if (error->code == error_none) {
        /* Initializes the new hash set's occupied flags. */
        for (i = 0; i < new_set.capacity; i++) {
            new_set.occupied[i] = false;
        }

        /* Reinserts entries from old hash set into the new hash set. */
        for (i = 0; i < set->capacity; i++) {
            if (set->occupied[i]) {
                hash = moviedb_id_hash(set->entries[i]);
                index = probe_index(&new_set, set->entries[i], hash);
                new_set.entries[index] = set->entries[i];
                new_set.occupied[index] = true;
            }
        }

        /* Frees the old hash set. */
        moviedb_free(set->occupied);
        moviedb_free(set->entries);
        set->capacity = new_set.capacity;
        set->entries = new_set.entries;
        set->occupied = new_set.occupied;
    }
}
