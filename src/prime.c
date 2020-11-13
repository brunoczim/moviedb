#include <stdint.h>
#include "prime.h"

bool is_prime(size_t number)
{
    size_t div, square, square_inc;
    bool is_prime, testing;

    /* Default case: not prime. */
    is_prime = false;

    if (number == 2) {
        /* Corner case, the even prime number. */
        is_prime = true;
    } else if (number % 2 != 0 && number > 2) {
        /* Only search if not an even number. */
        /* Start from 3. */
        div = 3;
        /* Squared divisor. */
        square = div * div;
        /* Increment to add to the square each iteration. */
        square_inc = 16;
        testing = true;

        /* Loop while div < sqrt(number) AND possibly prime. */
        while (testing) {
            if (square > number) {
                testing = false;
                is_prime = true;
            } if (number % div == 0) {
                testing = false;
            } else if (SIZE_MAX - square_inc < square) {
                testing = false;
                is_prime = true;
            } else {
                square += square_inc;
                square_inc += 8;

                if (SIZE_MAX - 2 < div) {
                    testing = false;
                    is_prime = true;
                } else {
                    div += 2;
                }
            }
        }
    }

    return is_prime;
}

size_t next_prime(size_t number)
{
    if (number <= 2) {
        /* Corner case of 0, 1, and 2. */
        number = 2;
    } else if (number % 2 == 0) {
        /* Even number, not 2, it will not be prime. Only odd numbers will. */
        number++;
    }

    while (!is_prime(number) && number < SIZE_MAX) {
        number += 2;
    }

    return number;
}
