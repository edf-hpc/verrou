#!/bin/bash

verrou --trace-children=yes ./dot.py > $1/out 2>&1
