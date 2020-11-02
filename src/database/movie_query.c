#include "../database.h"

/**
 * Appends a row to the query buffer.
 */
static void buf_append(
        struct movie_query_buf *restrict buf,
        struct movie const *row,
        struct error *restrict error);

/**
 * Sorts the query buffer by ID.
 */
static void sort_buf(struct movie_query_buf *restrict buf);

/**
 * Helper to find the parent of a given node index of a heap.
 */
static inline size_t sort_heap_parent(size_t child);

/**
 * Helper to find the left child of a given node index of a heap.
 */
static inline size_t sort_heap_left(size_t parent);

/**
 * Helper to find the right child of a given node index of a heap.
 */
static inline size_t sort_heap_right(size_t parent);

/**
 * Heapifies the buffer from the nodes above the given index.
 */
static void sort_heapify_above(
        struct movie_query_buf *restrict buf,
        size_t index);

/**
 * Heapifies the whole buffer.
 */
static void sort_heapify(struct movie_query_buf *restrict buf);

extern inline void movie_query_init(struct movie_query_buf *restrict buf);

void movie_query(
        struct database const *restrict database,
        char const *restrict prefix,
        struct movie_query_buf *restrict query_buf,
        struct error *restrict error)
{
    struct trie_iter iter;
    db_id_t movieid;
    struct movie const *movie;
    bool has_data;

    query_buf->length = 0;
    trie_search_prefix(&database->trie_root, prefix, &iter);

    do {
        has_data = trie_next_movie(&iter, &movieid, error);
        if (has_data) {
            movie = movies_search(&database->movies, movieid);
            if (movie != NULL) {
                buf_append(query_buf, movie, error);
                has_data = error->code == error_none;
            }
        }
    } while (has_data);

    trie_iter_destroy(&iter);

    if (error->code == error_none) {
        sort_buf(query_buf);
    }
}

void movie_query_print(struct movie_query_buf const *restrict query_buf)
{
    size_t i;

    for (i = 0; i < query_buf->length; i++) {
        movie_print(query_buf->rows[i]);
    }
}

extern inline void movie_query_destroy(struct movie_query_buf *restrict buf);

static void buf_append(
        struct movie_query_buf *restrict buf,
        struct movie const *row,
        struct error *restrict error)
{
    struct movie const **new_rows;
    size_t new_cap;

    if (buf->length == buf->capacity) {
        new_cap = buf->capacity * 2;
        if (new_cap == 0) {
            new_cap = 1;
        }
        new_rows = db_realloc(
                buf->rows,
                new_cap * sizeof (struct movie const *),
                error);

        if (error->code == error_none) {
            buf->rows = new_rows;
            buf->capacity = new_cap;
        }
    }

    if (error->code == error_none) {
        buf->rows[buf->length] = row;
        buf->length++;
    }
}

static inline size_t sort_heap_parent(size_t child)
{
    return (child - 1) / 2;
}

static inline size_t sort_heap_left(size_t parent)
{
    return parent * 2 + 1;
}

static inline size_t sort_heap_right(size_t parent)
{
    return parent * 2 + 2;
}

static void sort_buf(struct movie_query_buf *restrict buf)
{
    size_t i;
    struct movie const *tmp;

    sort_heapify(buf);

    i = buf->length;

    while (i > 1) {
        i--;
        tmp = buf->rows[0];
        buf->rows[0] = buf->rows[i];
        buf->rows[i] = tmp;
        sort_heapify_above(buf, i);
    }
}

static void sort_heapify(struct movie_query_buf *restrict buf)
{
    size_t i;

    for (i = 1; i < buf->length; i++) {
        sort_heapify_above(buf, i);
    }
}

static void sort_heapify_above(
        struct movie_query_buf *restrict buf,
        size_t index)
{
    size_t child, parent;
    bool is_correct;
    struct movie const *tmp;

    child = buf->length - 1;
    is_correct = false;

    while (child > index && !is_correct) {
        parent = sort_heap_parent(child);
        is_correct = buf->rows[parent]->id >= buf->rows[child]->id;
        if (!is_correct) {
            tmp = buf->rows[parent];
            buf->rows[parent] = buf->rows[child];
            buf->rows[child] = tmp;
            child = parent;
        }
    }
}
