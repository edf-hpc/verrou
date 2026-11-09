#!/usr/bin/env python3
"""Compare script associated with ddMetamodelRun.py"""
import sys
from pathlib import Path

if __name__=="__main__":
    ref=Path(sys.argv[1])
    toCmp=Path(sys.argv[2])
    if ref==toCmp:
        sys.exit(0)
    with open(str(toCmp / "dd.metamodel.fail")) as f:
        sys.exit(int(f.readline()))
