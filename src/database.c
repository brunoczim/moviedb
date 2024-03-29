#include "database.h"
#include "io.h"
#include "csv/movie.h"
#include "csv/rating.h"
#include "csv/tag.h"

#define IO_BUF_SIZE 0x10000

/**
 * Loads the data from the movie.csv file.
 */
static void load_movies(
        struct database *restrict database,
        struct strbuf *restrict buf,
        char *file_buf,
        struct error *restrict error);

/**
 * Loads the data from the rating.csv file.
 */
static void load_ratings(
        struct database *restrict database,
        struct strbuf *restrict buf,
        char *file_buf,
        struct error *restrict error);

/**
 * Loads the data from the tag.csv file.
 */
static void load_tags(
        struct database *restrict database,
        struct strbuf *restrict buf,
        char *file_buf,
        struct error *restrict error);

void database_load(
        struct database *restrict database_out,
        struct strbuf *restrict buf,
        struct error *restrict error)
{
    char *file_buf;

    trie_root_init(&database_out->trie_root);
    /* Initializes movies to capacity 2003. */
    movies_init(&database_out->movies, 2003, error);

    if (error->code == error_none) {
        /* Initializes users to capacity 2003. */
        users_init(&database_out->users, 2003, error);
    }

    if (error->code == error_none) {
        /* Initializes tags to capacity 2003. */
        tags_init(&database_out->tags, 2003, error);
    }

    if (error->code == error_none) {
        /* Allocates the buffer for file buffering. */
        file_buf = moviedb_alloc(sizeof(*file_buf), IO_BUF_SIZE, error);
    }

    /* Actually loads everything, if no error. */
    if (error->code == error_none)  {
        load_movies(database_out, buf, file_buf, error);

        if (error->code == error_none)  {
            load_ratings(database_out, buf, file_buf, error);
        }

        if (error->code == error_none)  {
            load_tags(database_out, buf, file_buf, error);
        }

        moviedb_free(file_buf);
    }
}

void database_destroy(struct database *restrict database)
{
    trie_destroy(&database->trie_root);
    movies_destroy(&database->movies);
    users_destroy(&database->users);
    tags_destroy(&database->tags);
}

static void load_movies(
        struct database *restrict database,
        struct strbuf *restrict buf,
        char *file_buf,
        struct error *restrict error)
{
    char const *path = "data/movie.csv";
    FILE *file;
    struct movie_parser parser;
    struct movie_csv_row row;
    bool has_data = true;

    file = input_file_open(path, error);

    if (error->code == error_none) {
        input_file_setbuf(file, file_buf, IO_BUF_SIZE, error);

        if (error->code == error_none) {
            /* Initializes the CSV parser. */
            movie_parser_init(&parser, file, buf, error);
        }

        has_data = error->code == error_none;
        while (has_data) {
            has_data = movie_row_parse(&parser, buf, &row, error);
            if (has_data) {
                /* Inserts into the trie. */
                trie_insert(&database->trie_root, row.title, row.id, error);

                if (error->code == error_dup_movie_title) {
                    /* Ignore duplicated movie title error. */
                    error_set_code(error, error_none);
                }

                if (error->code == error_none) {
                    /* Inserts into the movie table. */
                    movies_insert(&database->movies, &row, error);
                } 
                if (error->code != error_none) {
                    /*
                     * Destroys the row memory if an error happens.
                     *
                     * Normally, movie table would get the ownership of the
                     * heap-allocated fields of the row.
                     */
                    movie_row_destroy(&row);
                }

                has_data = error->code == error_none;
            }
        }

        input_file_close(file);
    }


    if (error->code != error_none) {
        error_set_context(error, path, false);
    }
}


static void load_ratings(
        struct database *restrict database,
        struct strbuf *restrict buf,
        char *file_buf,
        struct error *restrict error)
{
    char const *path = "data/rating.csv";
    FILE *file;
    struct rating_parser parser;
    struct rating_csv_row row;
    bool has_data = true;

    file = input_file_open(path, error);

    if (error->code == error_none) {
        input_file_setbuf(file, file_buf, IO_BUF_SIZE, error);

        if (error->code == error_none) {
            /* Initializes the CSV parser. */
            rating_parser_init(&parser, file, buf, error);
        }

        has_data = error->code == error_none;
        while (has_data) {
            has_data = rating_row_parse(&parser, buf, &row, error);

            if (has_data) {
                /* Inserts into the user table. */
                users_insert_rating(&database->users, &row, error);

                if (error->code == error_none) {
                    /* Adds this rating to the respective movie. */
                    movies_add_rating(
                            &database->movies,
                            row.movieid,
                            row.value);
                }

                has_data = error->code == error_none;
            }
        }

        input_file_close(file);
    }


    if (error->code != error_none) {
        error_set_context(error, path, false);
    }
}

static void load_tags(
        struct database *restrict database,
        struct strbuf *restrict buf,
        char *file_buf,
        struct error *restrict error)
{
    char const *path = "data/tag.csv";
    FILE *file;
    struct tag_parser parser;
    struct tag_csv_row row;
    bool has_data = true;

    file = input_file_open(path, error);

    if (error->code == error_none) {
        input_file_setbuf(file, file_buf, IO_BUF_SIZE, error);

        if (error->code == error_none) {
            /* Initializes the CSV parser. */
            tag_parser_init(&parser, file, buf, error);
        }

        has_data = error->code == error_none;
        while (has_data) {
            has_data = tag_row_parse(&parser, buf, &row, error);

            if (has_data) {
                /* Inserts into the tag table. */
                tags_insert(&database->tags, &row, error);
                if (error->code == error_dup_movie_id) {
                    /* Ignore duplicated movie ID error. */
                    error_set_code(error, error_none);
                }
                has_data = error->code == error_none;
            }
        }

        input_file_close(file);
    }

    if (error->code != error_none) {
        error_set_context(error, path, false);
    }
}
