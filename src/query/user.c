#include "user.h"
#include "../io.h"

/* Colors for the columns */
#define COLOR_USER_RATING TERMINAL_MAGENTA
#define COLOR_TITLE TERMINAL_GREEN
#define COLOR_GLOBAL_RATING TERMINAL_RED
#define COLOR_RATINGS TERMINAL_BLUE

void user_query_init(
        struct user_query_iter *restrict iter_out,
        struct database const *database,
        moviedb_id_t userid)
{
    iter_out->database = database;
    iter_out->user = users_search(&database->users, userid);
    iter_out->current = 0;
}

bool user_query_next(
        struct user_query_iter *restrict iter,
        struct user_query_row *restrict row_out)
{
    struct movie const *movie = NULL;
    struct user_rating const *rating = NULL;

    if (iter->user == NULL) {
        return false;
    }

    while (iter->current < iter->user->ratings.length && movie == NULL) {
        rating = &iter->user->ratings.entries[iter->current];
        movie = movies_search(&iter->database->movies, rating->movie);

        if (movie != NULL) {
            row_out->user_rating = rating->value;
            row_out->title = movie->title;
            row_out->global_rating = movie->mean_rating;
            row_out->ratings = movie->ratings;
        }

        iter->current++;
    }

    return movie != NULL;
}

void user_query_print_header(void)
{
    /* Puts the header by concatenating string literals. */
    puts(COLOR_USER_RATING "User Rating"
            TERMINAL_CLEAR ", "
            COLOR_TITLE "Movie Title"
            TERMINAL_CLEAR ", "
            COLOR_GLOBAL_RATING "Global Rating"
            TERMINAL_CLEAR ", "
            COLOR_RATINGS "Ratings Count"
            TERMINAL_CLEAR);
}

void user_query_print_row(struct user_query_row const *restrict row)
{
    printf(COLOR_USER_RATING "%.1lf"
            TERMINAL_CLEAR ", "
            COLOR_TITLE "%s"
            TERMINAL_CLEAR ", "
            COLOR_GLOBAL_RATING "%.1f"
            TERMINAL_CLEAR ", "
            COLOR_RATINGS "%zu"
            TERMINAL_CLEAR "\n",
            row->user_rating,
            row->title,
            row->global_rating,
            row->ratings);
}

void user_query_print(struct user_query_iter *restrict iter)
{
    struct user_query_row row;
    size_t count = 0;

    user_query_print_header();

    putchar('\n');

    while (user_query_next(iter, &row)) {
        user_query_print_row(&row);
        count++;
    }

    printf("\nFound %zu results\n", count);
}
