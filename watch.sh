#!/usr/bin/env sh

print_sep() {
    echo
    echo $'\e''[94m'"===="$'\e''[0m'
    echo
}

run_cmd() {
    if [ $# -ne 0 ]
    then
        "$@"
    else
        ./build.sh debug all
    fi
}

loop () {
    while read evt
    do
        print_sep
        run_cmd "$@"
    done
}

run_cmd "$@"

inotifywait -q -m -e close_write Makefile $(find src -type f) | loop "$@"
