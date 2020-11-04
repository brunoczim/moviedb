#include "../io.h"
#include "../database.h"

/* Colors for the columns */
#define COLOR_ID TERMINAL_MAGENTA
#define COLOR_TITLE TERMINAL_GREEN
#define COLOR_GENRES TERMINAL_YELLOW
#define COLOR_MEAN_RATING TERMINAL_RED
#define COLOR_RATINGS TERMINAL_BLUE

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
 * Heapifies the buffer from the nodes in the range [start, end).
 */
static void sort_heapify_range(
        struct movie_query_buf *restrict buf,
        size_t start,
        size_t end);

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

void movie_query_print_header(void)
{
    /* Puts the header by concatenating string literals. */
    puts(COLOR_ID "ID"
            TERMINAL_CLEAR ", "
            COLOR_TITLE "Title"
            TERMINAL_CLEAR ", "
            COLOR_GENRES "Genres"
            TERMINAL_CLEAR ", "
            COLOR_MEAN_RATING "Mean Rating"
            TERMINAL_CLEAR ", "
            COLOR_RATINGS "Ratings Count"
            TERMINAL_CLEAR);
}

void movie_query_print_row(struct movie const *restrict row)
{
    char id_buffer[MOVIEDB_ID_DIGITS + 1];
    size_t id_start;

    id_start = db_id_to_str(row->id, id_buffer, MOVIEDB_ID_DIGITS + 1);

    /* Puts the row by concatenating string literals. */
    printf(COLOR_ID "%s"
            TERMINAL_CLEAR ", "
            COLOR_TITLE "%s"
            TERMINAL_CLEAR ", "
            COLOR_GENRES "%s"
            TERMINAL_CLEAR ", "
            COLOR_MEAN_RATING "%.1lf"
            TERMINAL_CLEAR ", "
            COLOR_RATINGS "%zu"
            TERMINAL_CLEAR "\n",
            id_buffer + id_start,
            row->title,
            row->genres,
            row->mean_rating,
            row->ratings);
}


void movie_query_print(struct movie_query_buf const *restrict query_buf)
{
    size_t i;

    movie_query_print_header();

    putchar('\n');

    for (i = 0; i < query_buf->length; i++) {
        movie_query_print_row(query_buf->rows[i]);
    }
    
    printf("\nFound %zu results\n", query_buf->length);
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
        sort_heapify_range(buf, 0, i);
    }
}

static void sort_heapify(struct movie_query_buf *restrict buf)
{
    /* divide by 2 so we skip leaf nodes. */
    size_t node = buf->length / 2;

    while (node > 0) {
        node--;
        sort_heapify_range(buf, node, buf->length);
    }
}

static void sort_heapify_range(
        struct movie_query_buf *restrict buf,
        size_t start,
        size_t end)
{
    size_t max;
    size_t parent = start;
    size_t child = sort_heap_left(parent);
    struct movie const *tmp;
    bool is_correct = false;

    while (child < end && !is_correct) {
        max = parent;

        if (buf->rows[max]->id < buf->rows[child]->id) {
            max = child;
        }

        child += 1;

        if (child < end && buf->rows[max]->id < buf->rows[child]->id) {
            max = child;
        }

        is_correct = max == parent;

        if (!is_correct) {
            tmp = buf->rows[parent];
            buf->rows[parent] = buf->rows[max];
            buf->rows[max] = tmp;
            parent = max;
            child = sort_heap_left(parent);
        }
    }
}
