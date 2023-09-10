#!/bin/sh -xe

CC=gcc
CFLAGS="-std=c11 -Wall -Wextra -Wpedantic"

$CC -o easypdf_test easypdf_test.c easypdf.c $CFLAGS
