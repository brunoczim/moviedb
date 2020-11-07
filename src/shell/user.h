#ifndef MOVIEDB_SHELL_USER_H
#define MOVIEDB_SHELL_USER_H 1

#include "../shell.h"

/**
 * Runs the user command. The command searches for the ratings of a user.
 * Returns whether the shell should still execute. 
 */
bool shell_run_user(struct shell *restrict shell, struct error *restrict error);

#endif
