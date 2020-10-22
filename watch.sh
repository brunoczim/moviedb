#!/usr/bin/env sh

loop () {
    while read evt
    do
        echo
        echo Building in "$@" mode
        ./build.sh "$@"
    done
}

echo Building in "$@" mode
./build.sh "$@"

inotifywait -q -m -e close_write src/* Makefile | loop "$@"
