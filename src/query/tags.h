#ifndef TAGSDB_QUERY_TAGS_H
#define TAGSDB_QUERY_TAGS_H 1

#include "../database.h"

/**
 * This file declares utilities related to the 'tags' query.
 */

/**
 * The input of a tags query. More specifically, the tags used to search.
 */
struct tags_query_input {
    /**
     * The array of pointer to tags. Only internal tags query code is allowed to
     * touch this.
     */
    struct tag const **tags;
    /**
     * How many tags are there. Only internal tags query code is allowed to
     * touch this.
     */
    size_t length;
    /**
     * How many tags can be stored. Only internal tags query code is allowed to
     * touch this.
     */
    size_t capacity;
};

/**
 * Buffer used by the tags query.
 */
struct tags_query_buf {
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
     * How many rows can be stored. This likely won't change. Only internal
     * database code is allowed to touch this.
     */
    size_t capacity;
};

/**
 * Initializes the query input tags with the given capacity.
 */
void tags_query_input_init(
        struct tags_query_input *restrict query_input,
        size_t capacity,
        struct error *restrict error);

/**
 * Adds a tag to the query input, given its name.
 */
void tags_query_input_add(
        struct tags_query_input *restrict query_input,
        struct database const *restrict database,
        char const *restrict name,
        struct error *restrict error);

/**
 * Destroys the query input.
 */
inline void tags_query_input_destroy(
        struct tags_query_input *restrict query_input)
{
    moviedb_free(query_input->tags);
}

/**
 * Initializes the tags query buffer to the given capacity. The query will NOT
 * increase the capacity, it is intended to return only the N best.
 */
inline void tags_query_init(struct tags_query_buf *restrict buf)
{
    buf->rows = NULL;
    buf->capacity = 0;
    buf->length = 0;
}

/**
 * Performs the tags query. Searches for the best rated movies of the given
 * genre and with at least min_ratings count of ratings. The buffer must be
 * initalized and might be reused before being destroyed.
 */
void tags_query(
        struct database const *restrict database,
        struct tags_query_input const *restrict query_input,
        struct tags_query_buf *restrict query_buf,
        struct error *restrict error);

/**
 * Prints a tags query's header to the screen.
 */
void tags_query_print_header(void);

/**
 * Prints a tags query's row to the screen.
 */
void tags_query_print_row(struct movie const *restrict row);

/**
 * Prints a header and the rows found in the tags query.
 */
void tags_query_print(struct tags_query_buf const *restrict query_buf);

/**
 * Destroys the buffer of a tags query.
 */
inline void tags_query_destroy(struct tags_query_buf *restrict buf)
{
    moviedb_free(buf->rows);
}


#endif
