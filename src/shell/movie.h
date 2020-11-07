#ifndef MOVIEDB_SHELL_MOVIE_H
#define MOVIEDB_SHELL_MOVIE_H 1

#include "../shell.h"

/**
 * Reads a movie title prefix entered by the user and runs a search for movies
 * with that prefix in their names. Returns whether the shell should still
 * execute. Only shell internal code is allowed to touch this.
 */
bool shell_run_movie(
        struct shell *restrict shell,
        struct error *restrict error);

#endif
