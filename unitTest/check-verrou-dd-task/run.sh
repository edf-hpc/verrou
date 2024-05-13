#!/bin/bash


valgrind --tool=verrou --rounding-mode=random ./mainTime  > $1/res.out  

