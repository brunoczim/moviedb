#!/usr/bin/env sh

set -e

print_sep() {
    echo
    echo $'\e''[94m'"===="$'\e''[0m'
    echo
}

loop () {
    while read evt
    do
        print_sep "$@"
        "$@"
    done
}

"$@"

inotifywait -q -m -e close_write Makefile $(find src -type f) | loop "$@"
