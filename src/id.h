#ifndef MOVIEDB_ID_H
#define MOVIEDB_ID_H 1

#include "strbuf.h"

/**
 * This file provides items related to IDs in the application.
 */

/**
 * The type of any ID of this application.
 */
typedef long unsigned moviedb_id;

/**
 * Parses an ID from a given string buffer.
 */
moviedb_id moviedb_id_parse(struct strbuf *restrict buf, struct error *error);

#endif
