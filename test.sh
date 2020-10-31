#!/usr/bin/env sh

SUCCESS=true

print_sep() {
    echo
    echo $'\e''[94m'"===="$'\e''[0m'
    echo
}

run_test() {
    ./run.sh debug "test/$@" \
        && print_sep \
        && ./run.sh sanitize "test/$@" \
        && print_sep \
        && ./run.sh release "test/$@"
}

for TEST in csv trie prime movies_table users_table
do
    if ! run_test "$TEST"
    then
        SUCCESS=false
    fi
    print_sep
done

if $SUCCESS
then
    echo $'\e''[92m'"==== ALL TESTS PASSED ===="$'\e''[0m'
else
    echo $'\e''[91m'"==== FAILURE ===="$'\e''[0m'
fi
