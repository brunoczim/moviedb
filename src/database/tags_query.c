#include "../database.h"

/* Colors for the columns */
#define COLOR_TITLE TERMINAL_GREEN
#define COLOR_GENRES TERMINAL_YELLOW
#define COLOR_MEAN_RATING TERMINAL_RED
#define COLOR_RATINGS TERMINAL_BLUE

static void buf_append(
        struct tags_query_buf *restrict buf,
        struct movie const *movie,
        struct error *restrict error);

static bool movie_in_tags(
        struct tags_query_input const *restrict input,
        db_id_t movieid);

void tags_query_input_init(
        struct tags_query_input *restrict query_input,
        size_t capacity,
        struct error *restrict error)
{
    query_input->tags = db_alloc(sizeof(struct tag const *) * capacity, error);
    query_input->capacity = capacity;
    query_input->length = 0;
}

void tags_query_input_add(
        struct tags_query_input *restrict query_input,
        struct database const *restrict database,
        char const *restrict name,
        struct error *restrict error)
{
    size_t new_cap;
    struct tag const *tag, *tmp;
    struct tag const **new_tags;

    tag = tags_search(&database->tags, name);

    if (tag != NULL) {
        if (query_input->length == query_input->capacity) {
            new_cap = query_input->capacity * 2;
            if (new_cap == 0) {
                new_cap = 1;
            }

            new_tags = db_realloc(
                    query_input->tags,
                    sizeof(struct tag const *) * new_cap,
                    error);

            if (error->code == error_none) {
                query_input->tags = new_tags;
                query_input->capacity = new_cap;
            }
        }

        if (error->code == error_none) {
            query_input->tags[query_input->length] = tag;
            query_input->length++;

            if (query_input->tags[0]->movies.length > tag->movies.length) {
                tmp = query_input->tags[0];
                query_input->tags[0] = tag;
                query_input->tags[query_input->length - 1] = tmp;
            }
        }
    }
}


extern inline void tags_query_input_destroy(
        struct tags_query_input *restrict query_input);

extern inline void tags_query_init(struct tags_query_buf *restrict buf);

void tags_query(
        struct database const *restrict database,
        struct tags_query_input const *restrict query_input,
        struct tags_query_buf *restrict query_buf,
        struct error *restrict error)
{
    struct movie const *movie;
    struct tag_movies_iter iter;
    size_t i;
    db_id_t movieid;

    if (query_input->length > 0) {
        tag_movies_iter(&query_input->tags[0]->movies, &iter);
        
        while (error->code == error_none && tag_movies_next(&iter, &movieid)) {
            if (movie_in_tags(query_input, movieid)) {
                movie = movies_search(&database->movies, movieid);
                if (movie != NULL) {
                    buf_append(query_buf, movie, error);
                }
            }
            i++;
        }
    }
}

void tags_query_print_header(void)
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

void tags_query_print_row(struct movie const *restrict row)
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

void tags_query_print(struct tags_query_buf const *restrict query_buf)
{
    size_t i;

    tags_query_print_header();

    putchar('\n');

    for (i = 0; i < query_buf->length; i++) {
        tags_query_print_row(query_buf->rows[i]);
    }
    
    printf("\nFound %zu results\n", query_buf->length);
}

extern inline void tags_query_destroy(struct tags_query_buf *restrict buf);

static void buf_append(
        struct tags_query_buf *restrict buf,
        struct movie const *movie,
        struct error *restrict error)
{

    size_t new_cap;
    struct movie const **new_movies;

    if (movie != NULL) {
        if (buf->length == buf->capacity) {
            new_cap = buf->capacity * 2;
            if (new_cap == 0) {
                new_cap = 1;
            }

            new_movies = db_realloc(
                    buf->rows,
                    sizeof(struct movie const *) * new_cap,
                    error);

            if (error->code == error_none) {
                buf->rows= new_movies;
                buf->capacity = new_cap;
            }
        }

        if (error->code == error_none) {
            buf->rows[buf->length] = movie;
            buf->length++;
        }
    }
}

static bool movie_in_tags(
        struct tags_query_input const *restrict input,
        db_id_t movieid)
{
    size_t tag = 1;
    bool found = true;

    while (tag < input->length && found) {
        found = tag_movies_contain(&input->tags[tag]->movies, movieid);
        tag++;
    } 

    return found;
}