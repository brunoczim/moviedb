#ifndef MOVIEDB_SHELL_TOPN_H
#define MOVIEDB_SHELL_TOPN_H 1

#include "../shell.h"

/**
 * Runs the topN command. The command finds N movies with the best ratings of a
 * given genre. Returns whether the shell should still execute. Only shell
 * internal code is allowed to touch this.
 */
bool shell_run_topn(struct shell *restrict shell, struct error *restrict error);

#endif
