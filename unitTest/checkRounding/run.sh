#!/bin/sh

valgrind --tool=verrou ./checkRoundingNative valgrind > $1/out.std
