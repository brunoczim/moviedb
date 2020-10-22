#!/usr/bin/env sh

set -e

echoerr () {
    echo "$@" 1>&2
}

help() {
    echoerr Usage:
    echoerr "    $0 debug"
    echoerr "    $0 release"
    echoerr "    $0 sanitize"
}

if [ $# -ne 1 ]
then
    help
    exit 1
fi

case $1 in
    debug)
        make PROFILE=DEBUG
        ;;
    release)
        make PROFILE=RELEASE
        ;;
    sanitize)
        make PROFILE=SANITIZE
        ;;
    *)
        help
        exit 1
        ;;
esac
