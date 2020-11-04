#include "../database.h"

#define MIN_RATINGS 1000

/* Colors for the columns */
#define COLOR_TITLE TERMINAL_GREEN
#define COLOR_GENRES TERMINAL_YELLOW
#define COLOR_MEAN_RATING TERMINAL_RED
#define COLOR_RATINGS TERMINAL_BLUE

static size_t buf_search(struct topn_query_buf *restrict buf, double rating);

static void buf_insert(
        struct topn_query_buf *restrict buf,
        struct movie const *row,
        size_t pos);

void topn_query_init(
        struct topn_query_buf *restrict buf,
        size_t capacity,
        struct error *restrict error)
{
    buf->capacity = capacity;
    buf->length = 0;
    buf->rows = db_alloc(sizeof(struct movie const *) * buf->capacity, error);
}

void topn_query(
        struct database const *restrict database,
        char const *restrict genre,
        size_t min_ratings,
        struct topn_query_buf *restrict query_buf)
{
    size_t pos;
    struct movies_iter iter;
    struct movie const *movie;
    bool has_genre;

    query_buf->length = 0;

    if (query_buf->capacity > 0) {
        movies_iter(&database->movies, &iter);

        movie = movies_next(&iter);

        while (movie != NULL) {
            has_genre = movie_has_genre(movie, genre);
            if (has_genre && movie->ratings >= min_ratings) {
                pos = buf_search(query_buf, movie->mean_rating);

                if (pos < query_buf->capacity) {
                    buf_insert(query_buf, movie, pos);
                }
            }

            movie = movies_next(&iter);
        }
    }

}

void topn_query_print_header(void)
{
    /* Puts the header by concatenating string literals. */
    puts(COLOR_TITLE "Title"
            TERMINAL_CLEAR ", "
            COLOR_GENRES "Genres"
            TERMINAL_CLEAR ", "
            COLOR_MEAN_RATING "Mean Rating"
            TERMINAL_CLEAR ", "
            COLOR_RATINGS "Ratings Count"
            TERMINAL_CLEAR);
}

void topn_query_print_row(struct movie const *restrict row)
{
    /* Puts the row by concatenating string literals. */
    printf(COLOR_TITLE "%s"
            TERMINAL_CLEAR ", "
            COLOR_GENRES "%s"
            TERMINAL_CLEAR ", "
            COLOR_MEAN_RATING "%.1lf"
            TERMINAL_CLEAR ", "
            COLOR_RATINGS "%zu"
            TERMINAL_CLEAR "\n",
            row->title,
            row->genres,
            row->mean_rating,
            row->ratings);
}

void topn_query_print(struct topn_query_buf const *restrict query_buf)
{
    size_t i;

    topn_query_print_header();

    putchar('\n');

    for (i = 0; i < query_buf->length; i++) {
        topn_query_print_row(query_buf->rows[i]);
    }
    
    printf("\nFound %zu results\n", query_buf->length);
}

extern inline void topn_query_destroy(struct topn_query_buf *restrict buf);

static size_t buf_search(struct topn_query_buf *restrict buf, double rating)
{
    size_t low, mid, high;
    double stored_rating;
    bool found;

    found = false;
    low = 0;
    high = buf->length;

    while (low < high && !found) {
        mid = low + (high - low) / 2;

        stored_rating = buf->rows[mid]->mean_rating;

        if (stored_rating > rating) {
            low = mid + 1;
        } else if (stored_rating < rating) {
            high = mid;
        } else {
            low = mid;
            found = true;
        }
    }

    return low;
}

static void buf_insert(
        struct topn_query_buf *restrict buf,
        struct movie const *row,
        size_t pos)
{
    size_t i;

    if (buf->length < buf->capacity) {
        buf->length++;
    }

    for (i = buf->length - 1; i > pos; i--) {
        buf->rows[i] = buf->rows[i - 1];
    }

    buf->rows[pos] = row;
}
