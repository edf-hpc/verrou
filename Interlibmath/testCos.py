#!/usr/bin/env python3

import math
import sys
import numpy

x=float(sys.argv[1])
y=numpy.nextafter(x,math.inf)


for i in range(4):
    print("cos(x)-cos(x): ", math.cos(x)-math.cos(x))
    print("cos(x)-cos(y): ", math.cos(x)-math.cos(y))

