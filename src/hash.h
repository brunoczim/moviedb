#ifndef MOVIEDB_HASH_H
#define MOVIEDB_HASH_H 1

#include <stdint.h>

typedef uint_fast64_t moviedb_hash;

inline size_t moviedb_hash_to_index(
        moviedb_hash hash,
        moviedb_hash attempt,
        size_t size)
{
    moviedb_hash term0 = hash % size;
    moviedb_hash term1 = attempt % size;
    moviedb_hash term2 = (term1 * term1) % size;
    moviedb_hash sum0 = (term0 + term1) % size;
    
    return (sum0 + term2) % size;
}

moviedb_hash moviedb_hash_uint64(uint_fast64_t integer);

moviedb_hash moviedb_hash_str(char const *restrict string);

#endif
