#ifndef MOVIEDB_SHELL
#define MOVIEDB_SHELL 1

#include "error.h"
#include "database.h"

/**
 * This file defines the interface of the shell/console mode of the application.
 */

/**
 * Data shared by shell functions. Only internal shell code is allowed to touch
 * this.
 */
struct shell {
    /**
     * An immutable pointer to the database.
     */
    struct database const *restrict database;
    /**
     * Buffer used by the shell to read lines.
     */
    struct strbuf *restrict buf;
    /**
     * Last read character.
     */
    int curr_ch;
};


/**
 * Runs the shell for the given trie tree root.
 */
void shell_run(struct database const *restrict database,
        struct strbuf *restrict buf,
        struct error *restrict error);

/**
 * Skips whitespace in the user input. Only internal shell code is allowed to
 * touch this.
 */
void shell_skip_whitespace(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Reads and runs a command entered by the user. Returns whether the shell
 * should still execute. Only internal shell code is allowed to touch this.
 */
bool shell_run_cmd(struct shell *restrict shell, struct error *restrict error);

/**
 * Reads the operation name entered by the user such as "movie" or "exit.
 * Returns whether the shell should still execute. Only internal shell code is
 * allowed to touch this.
 */
bool shell_read_op(struct shell *restrict shell, struct error *restrict error);

/**
 * Reads the single argument of an operation. Only internal shell code is
 * allowed to touch this.
 */
void shell_read_single_arg(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Reads a quoted argument. I.e. an argument of the form 'abc'. Escapes \\, \n
 * \" and \'. Only internal shell code is allowed to touch this.
 */
void shell_read_quoted_arg(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Reads the end of a command. Sets an error if there is more arguments. Only
 * internal shell code is allowed to touch this.
 */
void shell_read_end(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Discards an entire line from the terminal input. Only internal shell code is
 * allowed to touch this.
 */
void shell_discard_line(
        struct shell *restrict shell,
        struct error *restrict error);

/**
 * Prints a help message to the user, showing all operations. Only internal
 * shell code is allowed to touch this.
 */
void shell_print_help(void);

#endif
