#include "hash.h"

extern inline size_t moviedb_hash_to_index(
        moviedb_hash_t hash,
        moviedb_hash_t attempt,
        size_t size);

moviedb_hash_t moviedb_hash_uint64(uint_fast64_t integer)
{
    uint_fast64_t hash = integer;

    /* xorshift algorithm */
    hash ^= hash << 13;
    hash ^= hash >> 7;
    hash ^= hash << 17;

    /* large integer prime multiplication. */
    hash *= 0x3F1F06B16A65D581ull;

    /* xorshift again */
    hash ^= hash << 13;
    hash ^= hash >> 7;
    hash ^= hash << 17;

    /* multiplication by prime again. */
    hash *= 0x452537355C0164DDull;

    return hash;
}

moviedb_hash_t moviedb_hash_str(char const *restrict string)
{
    size_t index = 0;
    moviedb_hash_t curr;
    moviedb_hash_t hash = 0xFAABA99974B79A53;

    while (string[index] != 0) {
        curr = (unsigned char) string[index];
        hash *= 0xC3FB39E53E91D33F;
        hash ^= curr * 0x1A3C2A17EE36142B;
        index++;
    }

    hash ^= moviedb_hash_uint64(index);

    return hash;
}
