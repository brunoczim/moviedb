#include "shell.h"

enum command_code {
    cmd_exit,
    cmd_movie
};

struct movie_cmd {
    char const *prefix;
};

union command_data {

};

void moviedb_shell(
        struct trie_node const *trie_root,
        struct error *fatal_error)
{
}
