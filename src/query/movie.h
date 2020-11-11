#ifndef MOVIEDB_QUERY_MOVIE_H
#define MOVIEDB_QUERY_MOVIE_H 1

#include "../database.h"

/**
 * This file declares utilities related to the 'movie' query.
 */

/**
 * Buffer to store the result of a movie query.
 */
struct movie_query_buf {
    /**
     * The pointer to pointers to rows, i.e. array of pointers to rows. Only
     * internal database code is allowed to write to this. Reading is fine.
     */
    struct movie const **rows;
    /**
     * How many rows the query returned. Only internal database code is allowed
     * to write to this. Reading is fine.
     */
    size_t length;
    /**
     * How many rows can be stored. Only internal database code is allowed to
     * touch this.
     */
    size_t capacity;
};

/**
 * Initializes a movie query's buffer.
 */
inline void movie_query_init(struct movie_query_buf *restrict buf)
{
    buf->rows = NULL;
    buf->length = 0;
    buf->capacity = 0;
}

/**
 * Executes a movie query. The movie query returns all the movies with the given
 * prefix in their names, sorted by ID. query_buf must be initialized, and
 * might even contain data, but the data will be overwritten with the query
 * result.
 */
void movie_query(
        struct database const *restrict database,
        char const *restrict prefix,
        struct movie_query_buf *restrict query_buf,
        struct error *restrict error);

/**
 * Prints a movie query's header to the screen.
 */
void movie_query_print_header(void);

/**
 * Prints a movie query's row to the screen.
 */
void movie_query_print_row(struct movie const *restrict row);

/**
 * Prints a header and the rows found in the movie query.
 */
void movie_query_print(struct movie_query_buf const *restrict query_buf);

/**
 * Destroys the movie query buffer.
 */
inline void movie_query_destroy(struct movie_query_buf *restrict buf)
{
    moviedb_free(buf->rows);
}

#endif
