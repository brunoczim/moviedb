#!/usr/bin/env sh

set -e

echoerr () {
    echo "$@" 1>&2
}

help() {
    echoerr Usage:
    echoerr "    $0 debug <TEST>"
    echoerr "    $0 release <TEST>"
}

if [ $# -ne 2 ]
then
    help
    exit 1
fi

case $1 in
    release)
        make "test/$2" PROFILE=RELEASE
        ;;
    debug)
        make "test/$2" PROFILE=DEBUG
        ;;
    *)
        help
        exit 1
        ;;
esac


echo
echo $'\e''[94m'"== TEST $2 =="$'\e''[0m'

"./build/$1/test/$2"
