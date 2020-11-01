#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "error.h"
#include "alloc.h"
#include "strbuf.h"
#include "io.h"
#include "csv/movie.h"
#include "csv/rating.h"
#include "csv/tag.h"
#include "trie.h"
#include "movies.h"
#include "users.h"
#include "tags.h"
#include "shell.h"

#define IO_BUF_SIZE 0x10000

void load_all(
        struct trie_node *restrict trie_root,
        struct movies_table *restrict movies,
        struct users_table *restrict users,
        struct tags_table *restrict tags,
        struct strbuf *restrict buf,
        struct error *restrict error);

void load_movies(
        struct trie_node *restrict trie_root,
        struct movies_table *restrict movies,
        struct strbuf *restrict buf,
        char *file_buf,
        struct error *restrict error);

void load_ratings(
        struct movies_table *restrict movies,
        struct users_table *restrict users,
        struct strbuf *restrict buf,
        char *file_buf,
        struct error *restrict error);

void load_tags(
        struct tags_table *restrict tags,
        struct strbuf *restrict buf,
        char *file_buf,
        struct error *restrict error);

int main(int argc, char const *argv[])
{
    int exit_code = 0;
    struct error error;
    struct trie_node trie_root;
    struct movies_table movies;
    struct users_table users;
    struct tags_table tags;
    struct strbuf buf;

    error_init(&error);
    strbuf_init(&buf);
    trie_root_init(&trie_root);
    movies_init(&movies, 2003, &error);

    if (error.code == error_none) {
        users_init(&users, 2003, &error);
    }

    if (error.code == error_none) {
        tags_init(&tags, 2003, &error);
    }

    if (error.code == error_none) {
        load_all(&trie_root, &movies, &users, &tags, &buf, &error);
    }

    if (error.code == error_none) {
        shell_run(&trie_root, &movies, &buf, &error);
    }

    if (error.code != error_none) {
        error_print(&error);
    }

    users_destroy(&users);
    movies_destroy(&movies);
    trie_destroy(&trie_root);
    strbuf_destroy(&buf);
    error_destroy(&error);

    return exit_code;
}

void load_all(
        struct trie_node *restrict trie_root,
        struct movies_table *restrict movies,
        struct users_table *restrict users,
        struct tags_table *restrict tags,
        struct strbuf *restrict buf,
        struct error *restrict error)
{
    clock_t then, now;
    double millis;
    char *file_buf;

    puts("Loading data...");

    then = clock();

    file_buf = db_alloc(IO_BUF_SIZE, error);
    if (error->code == error_none)  {
        load_movies(trie_root, movies, buf, file_buf, error);
        if (error->code == error_none)  {
            load_ratings(movies, users, buf, file_buf, error);
        }
        if (error->code == error_none)  {
            load_tags(tags, buf, file_buf, error);
        }
        free(file_buf);
    }
    now = clock();

    millis = (now - then) / (CLOCKS_PER_SEC / 1000.0);
    printf("Data loaded in %.3lf milliseconds\n", millis);
}

void load_movies(
        struct trie_node *restrict trie_root,
        struct movies_table *restrict movies,
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
                trie_insert(trie_root, row.title, row.id, error);

                if (error->code == error_dup_movie_title) {
                    error->code = error_none;
                }
                if (error->code == error_none) {
                    movies_insert(movies, &row, error);
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


void load_ratings(
        struct movies_table *restrict movies,
        struct users_table *restrict users,
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
                users_insert_rating(users, &row, error);
                if (error->code == error_none) {
                    movies_add_rating(movies, row.movieid, row.value);
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

void load_tags(
        struct tags_table *restrict tags,
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
                tags_insert(tags, &row, error);
                has_data = error->code == error_none;
            }
        }

        input_file_close(file);

    }

    if (error->code != error_none) {
        error_set_context(error, path, false);
    }
}
