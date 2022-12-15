#!/bin/sh

valgrind --tool=verrou $BIN_NAME >$1/out
