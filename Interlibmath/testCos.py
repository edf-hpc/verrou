#!/usr/bin/env python

import math
import sys

x=float(sys.argv[1])

for i in xrange(4):
    print("cos diff: ", math.cos(x)-math.cos(x))

