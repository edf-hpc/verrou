#!/bin/sh


cat $1/out.std | grep "testFma<float>" |cut -f 4 | cut -d " " -f 2
