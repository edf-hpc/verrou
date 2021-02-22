#!/bin/bash

valgrind --trace-children=yes --tool=verrou ./dot.py > $1/out 2>&1
