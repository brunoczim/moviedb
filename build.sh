#!/usr/bin/env sh

echoerr () {
    echo "$@" 1>&2
}

help() {
    echoerr Usage:
    echoerr "    $0 debug"
    echoerr "    $0 release"
}

if [ $# -ne 1 ]
then
    help
    exit 1
fi

case $1 in
    release)
        make PROFILE=RELEASE
        ;;
    debug)
        make PROFILE=DEBUG
        ;;
    *)
        help
        exit 1
        ;;
esac
