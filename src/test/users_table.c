#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include "../alloc.h"
#include "../users.h"
#include "../error.h"

/** 
 * Tests users hash table implementation.
 */

int main(int argc, char const *argv[])
{
    struct error error;
    struct rating_csv_row rating;
    struct user const *user;
    struct users_table table;

    error_init(&error);

    users_init(&table, 5, &error);
    assert(error.code == error_none);
    assert(table.length == 0);
    assert(table.capacity == 5);

    rating.userid = 123;
    rating.value = 3.5;
    rating.movieid = 101;
    users_insert_rating(&table, &rating, &error);
    assert(error.code == error_none);
    assert(table.length == 1);
    assert(table.capacity == 5);

    user = users_search(&table, 123);
    assert(user != NULL);
    assert(user->id == 123);

    rating.userid = 456;
    rating.value = 4.0;
    rating.movieid = 201;
    users_insert_rating(&table, &rating, &error);
    assert(error.code == error_none);
    assert(table.length == 2);
    assert(table.capacity == 5);

    rating.userid = 789;
    rating.value = 5.0;
    rating.movieid = 101;
    users_insert_rating(&table, &rating, &error);
    assert(error.code == error_none);
    assert(table.length == 3);
    assert(table.capacity == 11);

    user = users_search(&table, 123);
    assert(user != NULL);
    assert(user->id == 123);

    user = users_search(&table, 456);
    assert(user != NULL);
    assert(user->id == 456);

    rating.userid = 123;
    rating.value = 4.0;
    rating.movieid = 201;
    users_insert_rating(&table, &rating, &error);
    assert(error.code == error_none);
    assert(table.length == 3);
    assert(table.capacity == 11);

    rating.userid = 123;
    rating.value = 2.5;
    rating.movieid = 301;
    users_insert_rating(&table, &rating, &error);
    assert(error.code == error_none);
    assert(table.length == 3);
    assert(table.capacity == 11);

    user = users_search(&table, 456);
    assert(user != NULL);
    assert(user->id == 456);
    assert(user->ratings.length == 1);
    assert(user->ratings.entries[0].movie == 201);
    assert(fabs(user->ratings.entries[0].value - 4.0) < 0.000001);

    user = users_search(&table, 123);
    assert(user != NULL);
    assert(user->id == 123);
    assert(user->ratings.length == 3);
    assert(user->ratings.entries[0].movie == 101);
    assert(fabs(user->ratings.entries[0].value - 3.5) < 0.000001);
    assert(user->ratings.entries[1].movie == 201);
    assert(fabs(user->ratings.entries[1].value - 4.0) < 0.000001);
    assert(user->ratings.entries[2].movie == 301);
    assert(fabs(user->ratings.entries[2].value - 2.5) < 0.000001);

    users_destroy(&table);
    error_destroy(&error);

    puts("Ok");

    return 0;
}
