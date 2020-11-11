#ifndef MOVIEDB_HASH_H
#define MOVIEDB_HASH_H 1

#include <stddef.h>
#include <stdint.h>

typedef uint_fast64_t moviedb_hash_t;

inline size_t moviedb_hash_to_index(
        moviedb_hash_t hash,
        moviedb_hash_t attempt,
        size_t size)
{
    /* quadratic probing */
    moviedb_hash_t term0 = hash % size;
    moviedb_hash_t term1 = attempt % size;
    moviedb_hash_t term2 = (term1 * term1) % size;
    moviedb_hash_t sum0 = (term0 + term1) % size;
    
    return (sum0 + term2) % size;

    /* linear probing */
    /* return (hash % size + attempt % size) % size; */
}

moviedb_hash_t moviedb_hash_uint64(uint_fast64_t integer);

moviedb_hash_t moviedb_hash_str(char const *restrict string);

#endif
