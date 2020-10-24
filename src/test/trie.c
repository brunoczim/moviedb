#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "../trie.h"
#include "../error.h"

/** 
 * Tests trie implementation.
 */

int main(int argc, char const *argv[])
{
    struct error error;
    struct trie_node root;
    unsigned long movieid;

    error_init(&error);

    trie_root_init(&root);

    assert(!trie_search(&root, "pineapple", &movieid));

    trie_insert(&root, "pineapple", 123, &error);
    assert(error.code == error_none);
    assert(trie_search(&root, "pineapple", &movieid));
    assert(movieid == 123);
    assert(!trie_search(&root, "pinetree", &movieid));
    assert(!trie_search(&root, "pine", &movieid));

    trie_insert(&root, "pinetree", 456, &error);
    assert(error.code == error_none);
    assert(trie_search(&root, "pineapple", &movieid));
    assert(movieid == 123);
    assert(trie_search(&root, "pinetree", &movieid));
    assert(movieid == 456);
    assert(!trie_search(&root, "pine", &movieid));

    trie_insert(&root, "pine", 789, &error);
    assert(error.code == error_none);
    assert(trie_search(&root, "pineapple", &movieid));
    assert(movieid == 123);
    assert(trie_search(&root, "pinetree", &movieid));
    assert(movieid == 456);
    assert(trie_search(&root, "pine", &movieid));
    assert(movieid == 789);

    assert(!trie_search(&root, "banana", &movieid));
    assert(error.code == error_none);
    trie_insert(&root, "banana", 104, &error);
    assert(trie_search(&root, "banana", &movieid));
    assert(movieid == 104);
    assert(trie_search(&root, "pineapple", &movieid));
    assert(movieid == 123);

    trie_destroy(&root);

    puts("Ok");

    return 0;
}
