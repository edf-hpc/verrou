#!/bin/sh

valgrind --tool=verrou ./checkRounding valgrind > $1/out.std
