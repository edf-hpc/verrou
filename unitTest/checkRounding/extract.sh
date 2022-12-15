#!/bin/sh


cat $1/out.std | grep "testInc0d1<float>" |cut -f 4 | cut -d " " -f 2
