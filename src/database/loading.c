#include "../database.h"
#include "../io.h"
#include "../csv/movie.h"
#include "../csv/rating.h"
#include "../csv/tag.h"

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
    movies_init(&database_out->movies, 2003, error);

    if (error->code == error_none) {
        users_init(&database_out->users, 2003, error);
    }

    if (error->code == error_none) {
        tags_init(&database_out->tags, 2003, error);
    }

    if (error->code == error_none) {
        file_buf = db_alloc(IO_BUF_SIZE, error);
    }

    if (error->code == error_none)  {
        load_movies(database_out, buf, file_buf, error);

        if (error->code == error_none)  {
            load_ratings(database_out, buf, file_buf, error);
        }

        if (error->code == error_none)  {
            load_tags(database_out, buf, file_buf, error);
        }

        db_free(file_buf);
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
    char const *path = "movie.csv";
    FILE *file;
    struct movie_parser parser;
    struct movie_csv_row row;
    bool has_data = true;

    file = input_file_open(path, error);

    if (error->code == error_none) {
        input_file_setbuf(file, file_buf, IO_BUF_SIZE, error);

        if (error->code == error_none) {
            movie_parser_init(&parser, file, buf, error);
        }

        has_data = error->code == error_none;
        while (has_data) {
            has_data = movie_row_parse(&parser, buf, &row, error);
            if (has_data) {
                trie_insert(&database->trie_root, row.title, row.id, error);

                if (error->code == error_dup_movie_title) {
                    error->code = error_none;
                }
                if (error->code == error_none) {
                    movies_insert(&database->movies, &row, error);
                } 
                if (error->code != error_none) {
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
    char const *path = "rating.csv";
    FILE *file;
    struct rating_parser parser;
    struct rating_csv_row row;
    bool has_data = true;

    file = input_file_open(path, error);

    if (error->code == error_none) {
        input_file_setbuf(file, file_buf, IO_BUF_SIZE, error);

        if (error->code == error_none) {
            rating_parser_init(&parser, file, buf, error);
        }

        has_data = error->code == error_none;
        while (has_data) {
            has_data = rating_row_parse(&parser, buf, &row, error);

            if (has_data) {
                users_insert_rating(&database->users, &row, error);

                if (error->code == error_none) {
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
    char const *path = "tag.csv";
    FILE *file;
    struct tag_parser parser;
    struct tag_csv_row row;
    bool has_data = true;

    file = input_file_open(path, error);

    if (error->code == error_none) {
        input_file_setbuf(file, file_buf, IO_BUF_SIZE, error);

        if (error->code == error_none) {
            tag_parser_init(&parser, file, buf, error);
        }

        has_data = error->code == error_none;
        while (has_data) {
            has_data = tag_row_parse(&parser, buf, &row, error);

            if (has_data) {
                tags_insert(&database->tags, &row, error);
                has_data = error->code == error_none;
            }
        }

        input_file_close(file);

    }

    if (error->code != error_none) {
        error_set_context(error, path, false);
    }
}
