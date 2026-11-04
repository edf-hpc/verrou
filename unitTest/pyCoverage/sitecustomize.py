# Loaded by every python process started by 'make python-coverage' (this
# directory is put first in PYTHONPATH): start the coverage measurement
# configured by COVERAGE_PROCESS_START.
import os
import sys

# A python program run under valgrind is the tested program, not a verrou
# tool: do not measure it (it would only slow down the instrumented run)
if "vgpreload" not in os.environ.get("LD_PRELOAD", ""):
    try:
        import coverage
        coverage.process_startup()
    except ImportError:
        pass

# This file shadows the sitecustomize of the system: run it too
_here = os.path.dirname(os.path.abspath(__file__))
for _dir in sys.path:
    _file = os.path.join(_dir, "sitecustomize.py")
    if os.path.abspath(_dir) != _here and os.path.isfile(_file):
        with open(_file) as _f:
            exec(compile(_f.read(), _file, "exec"))
        break
