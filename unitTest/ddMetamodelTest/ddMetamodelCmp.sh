#!/bin/sh
# Compare script associated with ddMetamodelRun.py (shell : cheaper than a python process by sample)
[ "$1" = "$2" ] && exit 0
read fail < "$2/dd.metamodel.fail"
exit $fail
