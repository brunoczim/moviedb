#include "../io.h"
#include "../database.h"

void user_query_init(
        struct user_query_iter *restrict iter_out,
        struct database const *database,
        db_id_t userid)
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
    puts(TERMINAL_MAGENTA "User Rating"
            TERMINAL_CLEAR ", "
            TERMINAL_GREEN "Movie Title"
            TERMINAL_CLEAR ", "
            TERMINAL_YELLOW "Global Rating"
            TERMINAL_CLEAR ", "
            TERMINAL_RED "Ratings Count"
            TERMINAL_CLEAR);
}

void user_query_print_row(struct user_query_row const *restrict row)
{
    printf(TERMINAL_MAGENTA "%.1lf"
            TERMINAL_CLEAR ", "
            TERMINAL_GREEN "%s"
            TERMINAL_CLEAR ", "
            TERMINAL_YELLOW "%.1f"
            TERMINAL_CLEAR ", "
            TERMINAL_RED "%zu"
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
