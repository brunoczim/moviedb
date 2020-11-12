#include "users.h"
#include "prime.h"
#include "alloc.h"

#define MAX_LOAD 0.5

/**
 * Initializes a user by allocating a user in the heap.
 */
static struct user *user_init(
        struct rating_csv_row *restrict rating_row,
        struct error *restrict error);

/**
 * Inserts a rating in the given rating list.
 */
static void ratings_insert(
        struct user_rating_list *restrict ratings,
        struct user_rating const *restrict rating,
        struct error *restrict error);

/**
 * Probes the given table until the place where the given user ID should be
 * stored, given its hash. Returns the index of this place.
 */
static size_t probe_index(
        struct users_table const *restrict table,
        moviedb_id_t userid,
        moviedb_hash_t hash);

/**
 * Resizes the table to have at least double capacity.
 */
static void resize(
        struct users_table *restrict table,
        struct error *restrict error);

void users_init(
        struct users_table *restrict table,
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

void users_insert_rating(
        struct users_table *restrict table,
        struct rating_csv_row *restrict rating_row,
        struct error *restrict error)
{
    struct user_rating rating;
    double load;
    moviedb_hash_t hash;
    size_t index;

    load = (table->length + 1) / (double) table->capacity;
    if (load >= MAX_LOAD) {
        /* Resize if it would be above maximum load. */
        resize(table, error);
    }

    if (error->code == error_none) {
        hash = moviedb_id_hash(rating_row->userid);
        index = probe_index(table, rating_row->userid, hash);

        if (table->entries[index] == NULL) {
            /* No previous insert with the given ID. */
            table->entries[index] = user_init(rating_row, error);
            if (error->code == error_none) {
                table->length++;
            }
        } else {
            /* There was a previous insert with the given ID. */
            rating.movie = rating_row->movieid;
            rating.value = rating_row->value;
            ratings_insert(&table->entries[index]->ratings, &rating, error);
        }
    }
}

struct user const *users_search(
        struct users_table const *restrict table,
        moviedb_id_t userid)
{
    moviedb_hash_t hash = moviedb_id_hash(userid);
    size_t index = probe_index(table, userid, hash);
    return table->entries[index];
}

void users_destroy(struct users_table *restrict table)
{
    size_t i;

    /* Iterates through all entries to free their user's memories. */
    for (i = 0; i < table->capacity; i++) {
        if (table->entries[i] != NULL) {
            moviedb_free(table->entries[i]->ratings.entries);
            moviedb_free(table->entries[i]);
        }
    }

    moviedb_free(table->entries);
}

static struct user *user_init(
        struct rating_csv_row *restrict rating_row,
        struct error *restrict error)
{
    struct user *user = moviedb_alloc(sizeof(*user), 1, error);

    if (error->code == error_none) {
        /* Initializes the user. */
        user->id = rating_row->userid;
        user->ratings.length = 1;
        user->ratings.capacity = 1;
        user->ratings.entries = moviedb_alloc(
                sizeof(*user->ratings.entries),
                user->ratings.capacity,
                error);

        /* Initializes the user's rating list with the given rating. */
        if (error->code == error_none) {
            user->ratings.entries[0].value = rating_row->value;
            user->ratings.entries[0].movie = rating_row->movieid;
        } else {
            moviedb_free(user);
            user = NULL;
        }
    }

    return user;
}

static void ratings_insert(
        struct user_rating_list *restrict ratings,
        struct user_rating const *restrict rating,
        struct error *restrict error)
{
    struct user_rating *new_entries;
    size_t new_cap;

    if (ratings->length == ratings->capacity) {
        /* Doubles capacity. 0 capacity never happens. */
        new_cap = ratings->capacity * 2;
        new_entries = moviedb_realloc(
                ratings->entries,
                sizeof(*new_entries),
                new_cap,
                error);

        if (error->code == error_none) {
            ratings->entries = new_entries;
            ratings->capacity = new_cap;
        }
    }

    if (error->code == error_none) {
        /* Finally inserts the rating. */
        ratings->entries[ratings->length] = *rating;
        ratings->length++;
    }
}

static size_t probe_index(
        struct users_table const *restrict table,
        moviedb_id_t userid,
        moviedb_hash_t hash)
{
    moviedb_hash_t attempt;
    size_t index;
    struct user *user;

    attempt = 0;
    index = moviedb_hash_to_index(hash, attempt, table->capacity);
    user = table->entries[index];

    /* Iterates while there is an user and it is not our target. */
    while (user != NULL && user->id != userid) {
        /*
         * If we reached here, the condition failed, and we need to get the
         * next attempt.
         */
        attempt++;
        index = moviedb_hash_to_index(hash, attempt, table->capacity);
        user = table->entries[index];
    }

    return index;
}

static void resize(
        struct users_table *restrict table,
        struct error *restrict error)
{
    size_t i;
    moviedb_hash_t hash;
    size_t index;
    struct users_table new_table;

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
