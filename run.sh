#!/usr/bin/env bash
name=${1%.c}
shift
make build/$name && exec build/$name "$@"
