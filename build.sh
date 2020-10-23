#!/usr/bin/env sh

set -e

echoerr () {
    echo "$@" 1>&2
}

help() {
    echoerr Usage:
    echoerr "    $0 debug [TARGET]"
    echoerr "    $0 release [TARGET]"
    echoerr "    $0 sanitize [TARGET]"
    echoerr
    echoerr "TARGET is optonal (default moviedb)"
}

if [ $# -eq 1 ]
then
    TARGET=moviedb
elif [ $# -eq 2 ]
then
    TARGET="$2"
else
    help
    exit 1
fi

case $1 in
    debug)
        make "$TARGET" PROFILE=DEBUG
        ;;
    release)
        make "$TARGET" PROFILE=RELEASE
        ;;
    sanitize)
        make "$TARGET" PROFILE=SANITIZE
        ;;
    *)
        help
        exit 1
        ;;
esac
