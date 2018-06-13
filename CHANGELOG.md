# Change Log

All notable changes between released versions of this project should be
documented here.

This project adheres to [Semantic Versioning](http://semver.org/).



## [UNRELEASED]

This version is based on Valgrind-3.13.0.

### Added

- Generation of Valgrind errors for NaN values. This can be useful to debug
  programs in conjunction with vgdb.
  
- Instrumentation of all FP binary instructions, as obtained by any combination of:
  - an operation:     ADD / SUB / MUL / DIV
  - a vector variant: LLO / SSE / AVX2
  - a precision:      single / double
  
- Instrumentation of cast instructions (double -> float).

- Preparation for the common interflop backend interface.

- (Experimental) Parallelism in `verrou_dd`. The number of concurrent threads
  can be set using the `VERROU_DD_NUM_THREADS` environnement variable.
  
- (Experimental) New DDmin algorithm for `verrou_dd`. Instead of computing a
  maximal set of stable symbols/lines using the DDmaw algorithm (and outputting
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

- More user-friendly verrou_dd messages.


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
