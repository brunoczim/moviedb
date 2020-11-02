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

/**
 * Number of maximum decimal digits in an ID.
 */
#define MOVIEDB_ID_DIGITS 20

/**
 * Parses an ID from a given string buffer.
 */
db_id_t db_id_parse(char const *restrict string, struct error *restrict error);

/**
 * Converts an ID to a string, using decimal digits. Returns the index where the
 * ID string starts on the buffer. It is recommended to reserve as much as
 * MOVIEDB_ID_DIGITS + 1 bytes for the buffer.
 */
size_t db_id_to_str(db_id_t id, char *restrict buffer, size_t buf_size);

/**
 * Processes the given id into a hash, so that the bits get scrambled.
 */
inline db_hash_t db_id_hash(db_id_t id)
{
    return db_hash_uint64(id);
}

#endif
