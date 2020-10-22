#!/usr/bin/env sh

print_header () {
    echo $'\e''[94m'"== Building in "$@" mode =="$'\e''[0m'
}

loop () {
    while read evt
    do
        echo
        print_header "$@"
        ./build.sh "$@"
    done
}

print_header "$@"
./build.sh "$@"

inotifywait -q -m -e close_write src/* Makefile | loop "$@"
