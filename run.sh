#!/usr/bin/env sh

set -e

echoerr () {
    echo "$@" 1>&2
}

help() {
    echoerr Usage:
    echoerr "    $0 debug [PROGRAM]"
    echoerr "    $0 release [PROGRAM]"
    echoerr "    $0 sanitize [PROGRAM]"
    echoerr
    echoerr "PROGRAM is optional (default moviedb)"
}

if [ $# -eq 1 ]
then
    PROG="moviedb"
elif [ $# -eq 2 ]
then
    PROG="$2"
else
    help
    exit 1
fi

case $1 in
    debug)
        PROFILE=DEBUG
        ;;
    release)
        PROFILE=RELEASE
        ;;
    sanitize)
        PROFILE=SANITIZE
        ;;
    *)
        help
        exit 1
        ;;
esac

make "$PROG" PROFILE="$PROFILE"

"./build/$1/$PROG"
