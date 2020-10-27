#ifndef MOVIEDB_PRIME_H
#define MOVIEDB_PRIME_H 1

#include <stddef.h>
#include <stdbool.h>

/**
 * This file exports utility functions to handle prime numbers. More
 * specifically, they uses a size_t type, because the goal is to use these
 * functions to find prime capacities for hash tables.
 */

/**
 * Tests whether the given number is prime.
 */
bool is_prime(size_t number);

/**
 * Finds the smallest prime such that prime >= number.
 */
size_t next_prime(size_t number);

#endif
