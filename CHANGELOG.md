# Change Log

## [UNRELEASED]

This version is based on Valgrind-3.17.0.

### Added


### Changed


---

## v2.3.1 - 2021-05-31

This version is based on Valgrind-3.17.0.

### Added
 - [EXPERIMENTAL] verrou_dd_syncho
	- Enable temporal delta-debug
	- Enable delta-debug for python code
 - checkdenorm back-end
        - The option --rounding-mode=ftz enable mode flush to zero
 - delta-debug improvments
	- Options with cmd options and env variables.
	- Add symlink FullPerturbation and NoPerturbation
	- Use relative path to print delta-debug progress
	- Add --cache option
	- Add heuristics to improve delta-debug (--rddmin-heuristics-cache= and --rddmin-heuristics-rep= options)
	- Add option --res-with-all-samples to be able to compare estimator between ddmin-*
	- Add parallelism (for samples and dichotomy)

### Changed
	- The option --demangle=no is no more required for --gen-exclude, --exclude, --gen-source, --source  options  and so for the delta-debug process
	- License change : The verrou backends and python tools switch to LGPL v2.1. The frontend remains in GPL.

---

## v2.2.0 - 2020-01-08

This version is based on Valgrind-3.15.0.

### Added

- [EXPERIMENTAL] MCA back-ends
  - use with valgrind option `--backend=mcaquad`
  - deactivate support for MCA back-ends with configure switch `--enable-verrou-quad=no`

- [EXPERIMENTAL] code coverage generation (`--trace=FILENAME`)

- Generate the list of cancellations (`--cc-gen-source`)

### Changed

- Two scripts `verrou_dd_line` and `verrou_dd_sym` replace
  `verrou_dd`.

- Bug fix related to the random generator.

- Bug fix: use the PID to generate the search space name.  This allows
  using Delta-Debugging techniques with MPI programs.

- Bug fix: correctly handle unnamed objects and source file lines.

- Bug fix: allow gdb integration. (fixes gh-24)

---

## v2.1.0 - 2018-11-09

This version is based on Valgrind-3.14.0. (fixes gh-19)

### Added

- Preliminary filtering before Delta-Debugging: only functions performing
  floating-point operations are considered in the search for
  instabilities.

- Multiple variants of the Delta-Debugging algorithm: (fixes gh-14, gh-22)
  - srDDmin: variant of rDDmin, specifically tuned to accomodate for stochastic
    tests
  - drDDmin: variant of rDDmin where a preliminary binary search is performed in
    order to further reduce the search space.

- New reduced-precision backend (`--rounding-mode=float`). This back-end
  emulates the use of single-precision arithmetic for all double-precision
  variables. (fixes gh-11)

### Changed

- Python3 port of `verrou_dd`.


---

## v2.0.0 - 2018-06-19

This version is based on Valgrind-3.13.0.

### Added

- Generation of Valgrind errors for NaN values. This can be useful to debug
  programs in conjunction with vgdb. (fixes gh-4)
  
- Instrumentation of all FP binary instructions, as obtained by any combination of:
  - an operation:     ADD / SUB / MUL / DIV
  - a vector variant: LLO / SSE / AVX2
  - a precision:      single / double
  
- Instrumentation of cast instructions (double -> float).

- Preparation for the common interflop backend interface.

- (Experimental) Parallelism in `verrou_dd`. The number of concurrent threads
  can be set using the `VERROU_DD_NUM_THREADS` environnement variable. (related
  to gh-7)
  
- (Experimental) New DDmin algorithm for `verrou_dd`. Instead of computing a
  maximal set of stable symbols/lines using the DDmax algorithm (and outputting
  the complement), the DDmin algorithm computes the union of minimal sets of
  unstable symbols/lines. It can be activated by setting `VERROU_DD_ALGO=rddmin`
  in the environment.


### Changed

- C++ source files are now compiled using the C++11 standard.

- Fix the LLO instrumentation bug. This solves problems which sometimes happened
  when LLO and real vector instructions were mixed. Known examples of such bugs
  situations include openblas or binaries compiled with the Intel compiler.
  
  The new --vr-unsafe-llo-optim allows keeping the old fast and buggy LLO
  instrumentation.

- Suppression of useless warnings.

- Fix bugs of next_after and next_prev.

- More robust rounding mode (upward, downward, toward_zero) with subnormals.

- Improvement of tests and unit test (with UCB references).

- Usability improvements of verrou_dd. There is no need for absolute paths to
  argument scripts anymore. Also, error messages are now more readable. (related
  to gh-7)

- Increase the max. size of symbol names in exclude files. New limit is set to
  4096 characters. (fixes gh-6)


---

## v1.1.0 - 2017-06-19

This version is based on Valgrind-3.13.0.

### Added

- Command-line option `--vr-seed` allows setting the pRNG seed in order to
  reproduce results in `random` and `average` rounding modes.


---

## v1.0.0 - 2017-05-19

This version is based on Valgrind-3.12.0.

### Added

- Continuous integration using the Travis system.
- Improve Delta-Debugging customization through environment variables.

### Changed

- There is no need anymore for an external, statically compiled libc/libm.


---

## v0.9.0 - 2017-03-31

This is the first released version of Verrou. It is based on Valgrind-3.10.1,
which supports the following system configurations:

- `gcc`, versions 3.x to 5.x
- `clang`, versions 2.9 to 4.x
- `glibc`, versions 2.2 to 2.20
