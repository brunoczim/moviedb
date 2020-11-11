#ifndef USERDB_QUERY_USER_H
#define USERDB_QUERY_USER_H 1

#include "../database.h"

/**
 * This file declares utilities related to the 'user' query.
 */

/**
 * A row of the "user" query.
 */
struct user_query_row {
    /**
     * The rating given by the specific user.
     */
    double user_rating;
    /**
     * Title of the rated movie.
     */
    char const *title;
    /**
     * Global rating of the rated movie, i.e. mean of all rates given to the
     * movie.
     */
    double global_rating;
    /**
     * Number of ratings given to the movie.
     */
    size_t ratings;
};

/**
 * Iterator over the rows of a "user" query.
 */
struct user_query_iter {
    /**
     * The database being looked at. Only internal database code is allowed to
     * touch this.
     */
    struct database const *database;
    /**
     * The user whose ratings are being iterated over. Only internal database
     * code is allowed to touch this.
     */
    struct user const *user;
    /**
     * Current rating being iterated. Only internal database code is allowed to
     * touch this.
     */
    size_t current;
};

/**
 * Initializes the user query's row iterator.
 */
void user_query_init(
        struct user_query_iter *restrict iter_out,
        struct database const *database,
        moviedb_id_t userid);

/**
 * Builds the next row and returns whether there is a next row. If there is a
 * next row, it is placed at the row_out parameter.
 */
bool user_query_next(
        struct user_query_iter *restrict iter_out,
        struct user_query_row *restrict row_out);

/**
 * Prints a user query's header to the screen.
 */
void user_query_print_header(void);

/**
 * Prints a user query's row to the screen.
 */
void user_query_print_row(struct user_query_row const *restrict row);

/**
 * Iterates through the user query rows and print them. Prints a header too.
 */
void user_query_print(struct user_query_iter *restrict iter);

#endif
