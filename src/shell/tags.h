#ifndef MOVIEDB_SHELL_TAGS_H
#define MOVIEDB_SHELL_TAGS_H 1

#include "../shell.h"

/**
 * Runs the "tags" command. Searches for movies with all the given tags. Only
 * shell internal code is allowed to touch this.
 */
bool shell_run_tags(struct shell *restrict shell, struct error *restrict error);

#endif
