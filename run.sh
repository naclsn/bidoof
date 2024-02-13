#!/bin/sh
test -d build/ || mkdir build/
name=${1%.c}; shift
make build/$name && exec build/$name "$@"
