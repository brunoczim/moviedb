#include "users.h"
#include "prime.h"
#include "alloc.h"

#define MAX_LOAD 0.5

/**
 * Initializes a user by allocating a user in the heap.
 */
static struct user *user_init(
        struct rating_csv_row *restrict rating,
        struct error *restrict error);

/**
 * Inserts a rating in the given rating list.
 */
static void ratings_insert(
        struct user_rating_list *restrict ratings,
        struct user_rating const *restrict rating,
        struct error *restrict error);

/**
 * Converts a hash to an index in the table.
 */
static size_t hash_to_index(
        struct users_table const *restrict table,
        uint_fast64_t hash,
        uint_fast64_t attempt);

/**
 * Probes the given table until the place where the given movie ID should be
 * stored, given its hash. Returns the index of this place.
 */
static size_t probe_index(
        struct users_table const *restrict table,
        moviedb_id userid,
        uint_fast64_t hash);

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
            sizeof(struct user **) * table->capacity,
            error);

    if (error->code == error_none) {
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
    uint_fast64_t hash;
    size_t index;

    load = (table->length + 1) / (double) table->capacity;
    if (load >= MAX_LOAD) {
        resize(table, error);
    }
        

    hash = moviedb_id_hash(rating_row->userid);
    index = probe_index(table, hash, rating_row->userid);

    if (table->entries[index] == NULL) {
        if (error->code == error_none) {
            table->entries[index] = user_init(rating_row, error);
            table->length++;
        }
    } else {
        rating.movie = rating_row->movieid;
        rating.value = rating_row->value;
        ratings_insert(&table->entries[index]->ratings, &rating, error);
    }
}

struct user const *users_search(
        struct users_table const *restrict table,
        moviedb_id userid)
{
    uint_fast64_t hash = moviedb_id_hash(userid);
    size_t index = probe_index(table, userid, hash);
    return table->entries[index];
}

void users_destroy(struct users_table *restrict table)
{
    size_t i;

    for (i = 0; i < table->capacity; i++) {
        if (table->entries[i] != NULL) {
            moviedb_free(table->entries[i]->ratings.entries);
            moviedb_free(table->entries[i]);
        }
    }

    moviedb_free(table->entries);
}

static struct user *user_init(
        struct rating_csv_row *restrict rating,
        struct error *restrict error)
{
    struct user *user = moviedb_alloc(sizeof(struct user), error);

    if (error->code == error_none) {
        user->id = rating->userid;
        user->ratings.length = 1;
        user->ratings.capacity = 1;
        user->ratings.entries = moviedb_alloc(
                sizeof(struct user_rating) * user->ratings.capacity,
                error);

        if (error->code == error_none) {
            user->ratings.entries[0].value = rating->value;
            user->ratings.entries[0].movie = rating->movieid;
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
    size_t new_cap = ratings->capacity * 2;

    if (ratings->length == ratings->capacity) {
        new_entries = moviedb_realloc(
                ratings->entries,
                sizeof(struct user_rating) * new_cap,
                error);

        if (error->code == error_none) {
            ratings->entries = new_entries;
            ratings->capacity = new_cap;
        }
    }

    if (error->code == error_none) {
        ratings->entries[ratings->length] = *rating;
        ratings->length++;
    }
}

static size_t hash_to_index(
        struct users_table const *restrict table,
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
        struct users_table const *restrict table,
        moviedb_id userid,
        uint_fast64_t hash)
{
    uint_fast64_t attempt;
    size_t index;
    struct user *user;

    attempt = 0;
    index = hash_to_index(table, hash, attempt);
    user = table->entries[index];

    while (user != NULL && user->id != userid) {
        attempt++;
        index = hash_to_index(table, hash, attempt);
        user = table->entries[index];
    }

    return index;
}

static void resize(
        struct users_table *restrict table,
        struct error *restrict error)
{
    size_t i;
    uint_fast64_t hash;
    size_t index;
    struct users_table new_table;

    if (SIZE_MAX / 2 >= table->capacity) {
        new_table.capacity = next_prime(table->capacity * 2);
    } else {
        new_table.capacity = SIZE_MAX;
    }

    new_table.entries = moviedb_alloc(
            sizeof(struct user **) * new_table.capacity,
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
