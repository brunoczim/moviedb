#ifndef MOVIEDB_ID_DEF_H
#define MOVIEDB_ID_DEF_H 1

#include <stdint.h>

/**
 * This file just defines an ID. Used to break cyclic header dependency.
 */

/**
 * The type of any ID of this application.
 */
typedef uint_least64_t db_id_t;

#endif
