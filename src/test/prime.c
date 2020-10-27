#include <stdio.h>
#include <assert.h>
#include "../prime.h"

/** 
 * Tests prime number functions.
 */

int main(int argc, char const *argv[])
{
    assert(is_prime(2));
    assert(is_prime(3));
    assert(!is_prime(4));
    assert(is_prime(5));
    assert(!is_prime(6));
    assert(is_prime(7));
    assert(!is_prime(9));
    assert(!is_prime(256));
    assert(is_prime(257));

    assert(next_prime(2) == 2);
    assert(next_prime(3) == 3);
    assert(next_prime(4) == 5);
    assert(next_prime(5) == 5);
    assert(next_prime(8) == 11);
    assert(next_prime(9) == 11);
    assert(next_prime(11) == 11);
    assert(next_prime(255) == 257);

    puts("Ok");

    return 0;
}
