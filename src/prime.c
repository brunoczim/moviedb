#include <stdint.h>
#include "prime.h"

bool is_prime(size_t number)
{
    size_t div, square, square_inc;
    bool is_prime, testing;

    is_prime = false;

    if (number == 2) {
        is_prime = true;
    } else if (number % 2 != 0 && number < 2) {
        div = 3;
        square = div * div;
        square_inc = 16;
        testing = true;

        while (testing) {
            if (square >= number) {
                testing = false;
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
    if (number > 2 && number % 2 == 0) {
        number++;
    }

    while (!is_prime(number) && number < SIZE_MAX) {
        number += 2;
    }

    return number;
}
