#!/bin/sh -xe

CC=gcc
CFLAGS="-std=c11 -ggdb -Wall -Wextra -Wpedantic"

$CC -o easypdf_test easypdf_test.c easypdf.c $CFLAGS
