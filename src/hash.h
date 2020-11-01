#ifndef MOVIEDB_HASH_H
#define MOVIEDB_HASH_H 1

#include <stddef.h>
#include <stdint.h>

typedef uint_fast64_t db_hash_t;

inline size_t db_hash_to_index(
        db_hash_t hash,
        db_hash_t attempt,
        size_t size)
{
    db_hash_t term0 = hash % size;
    db_hash_t term1 = attempt % size;
    db_hash_t term2 = (term1 * term1) % size;
    db_hash_t sum0 = (term0 + term1) % size;
    
    return (sum0 + term2) % size;
}

db_hash_t db_hash_uint64(uint_fast64_t integer);

db_hash_t db_hash_str(char const *restrict string);

#endif
