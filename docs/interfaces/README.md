# Interfaces of verrou

Reference of the interfaces that a new implementation of verrou must keep
(other instrumentation engine, refactoring):

- [inventory.md](inventory.md): options and environment variables of the
  verrou tool, client requests (ABI of `verrou.h`) and rounding mode values,
  monitor commands, IOMatch keys and actions, options of the python tools,
  existing deprecations. Generated from the sources by
  [gen_inventory.py](gen_inventory.py): run it after a change of an
  interface, `gen_inventory.py --check` (run by `unitTest/clientRequestAPI`)
  fails when the file is not up to date.
- [file-formats.md](file-formats.md): files read and written by verrou and the
  `verrou_dd_*` tools.
- the control protocol of `verrou_control` is specified in
  `pyTools/verrouControlProtocol.py`.

## ABI of `verrou.h`

The client requests are frozen: their numbers (order of the enum
`Vg_VerrouClientRequest`), their arguments and the values they return do not
change; a new request is added at the end of the enum, and increments
`VERROU_API_VERSION`. `VERROU_GET_API_VERSION` returns the version of the
running verrou (0 without verrou, or with a verrou older than version 1), so
that a program can check that a request exists before using it. The values of
enum `vr_RoundingMode` (returned by `VERROU_GET_ROUNDING`) are part of the
ABI: a new rounding mode is added at the end.

`verrou.h` needs no definition from the program (C and C++).

## Usages through gdb

With `--vgdb=yes`, gdb (or `vgdb`) sends the monitor commands of
[inventory.md](inventory.md) (`monitor status back`, `monitor rounding
UPWARD`...), with the Valgrind gdbserver commands (`v.kill`, ...). The verrou
errors (NaN, Inf, cancellation...) stop the program under gdb as any Valgrind
error (`--vgdb-error=`).
