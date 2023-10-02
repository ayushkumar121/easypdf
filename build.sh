#!/bin/sh -xe

CC=clang
CFLAGS="-std=c11 -ggdb -Wall -Wextra -Wpedantic -fsanitize=address"

$CC -o easypdf_test easypdf_test.c easypdf.c arena.c $CFLAGS
