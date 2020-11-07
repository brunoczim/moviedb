#!/usr/bin/env sh

set -e

echoerr () {
    echo "$@" 1>&2
}

help() {
    echoerr Usage:
    echoerr "    $0"
    echoerr "    $0 debug [PROGRAM]"
    echoerr "    $0 release [PROGRAM]"
    echoerr "    $0 sanitize [PROGRAM]"
    echoerr
    echoerr "PROGRAM is optional (default moviedb)"
    echoerr
    echoerr "If no arguments are given, \`$0 release moviedb\` is executed"
}

if [ $# -ge 2 ]
then
    help
    exit 1
fi

if [ $# -eq 0 ]
then
    MODE=release
else
    MODE="$1"
fi

if [ $# -le 1 ]
then
    PROG="moviedb"
else
    PROG="$2"
fi

case "$MODE" in
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

make "$PROG" PROFILE="$PROFILE" > /dev/null

"./build/$MODE/$PROG"
