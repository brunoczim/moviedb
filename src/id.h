#ifndef MOVIEDB_ID_H
#define MOVIEDB_ID_H 1

#include "id/def.h"
#include "strbuf.h"
#include "hash.h"

/**
 * This file provides items related to IDs in the application.
 */

/**
 * Number of bits in an ID.
 */
#define MOVIEDB_ID_BITS 64

#define MOVIEDB_ID_DIGITS 20

/**
 * Parses an ID from a given string buffer.
 */
db_id_t db_id_parse(char const *restrict string, struct error *restrict error);

/**
 * Processes the given id into a hash, so that the bits get scrambled.
 */
inline db_hash_t db_id_hash(db_id_t id)
{
    return db_hash_uint64(id);
}

#endif
