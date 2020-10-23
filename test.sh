#!/usr/bin/env sh

set -e

print_sep() {
    echo
    echo $'\e''[94m'"===="$'\e''[0m'
    echo
}

./run.sh debug test/csv

print_sep

./run.sh sanitize test/csv

print_sep

./run.sh release test/csv
