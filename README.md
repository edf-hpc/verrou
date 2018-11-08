# Verrou [![Build Status](https://travis-ci.org/edf-hpc/verrou.svg?branch=master)](https://travis-ci.org/edf-hpc/verrou)

Verrou helps you look for floating-point round-off errors in programs. It
implements various forms of arithmetic, including:

- all IEEE-754 standard rounding modes;

- two variants of stochastic floating-point arithmetic based on random rounding:
  all floating-point operations are perturbed by randomly switching rounding
  modes. These can be seen as an asynchronous variant of the CESTAC method, or a
  subset of Monte Carlo Arithmetic, performing only output randomization through
  random rounding;

- an emulation of single-precision rounding, in order to test the effect of
  reduced precision without any need to change the source code.

Verrou also comes with a `verrou_dd` utility, which simplifies the Verrou-based
debugging process by implementing several variants of the Delta-Debugging
algorithm. This allows easily locating which parts of the analyzed source code
are likely to be responsible for Floating-Point-related instabilities.

The documentation for Verrou is available as a dedicated [chapter in the
Valgrind manual](http://edf-hpc.github.io/verrou/vr-manual.html).

<p>&nbsp</p>

**NB:** This is released version 2.1.0 of Verrou, based on Valgrind v3.14.0. The
development version of Verrou can always be found in the
[`master`](https://github.com/edf-hpc/verrou/) branch. For other versions,
please consult the list of
[releases](https://github.com/edf-hpc/verrou/releases).



## Installation

### Get the sources

The preferred way to get Verrou sources is to download the latest *stable*
version: [v2.1.0](https://github.com/edf-hpc/verrou/releases/tag/v2.1.0).
Older versions are available in the [releases](https://github.com/edf-hpc/verrou/releases)
page. After downloading one of the released versions, skip to the "Configure
and build" section below.

<p>&nbsp;</p>

In order to build the *development* version of Verrou, it is necesary to first
download a specific Valgrind version, and patch it. Fetch valgrind's sources:

    git clone --branch=VALGRIND_3_14_0 --single-branch git://sourceware.org/git/valgrind.git valgrind-3.14.0+verrou-dev

Add verrou's sources to it:

    cd valgrind
    git clone --branch=master --single-branch https://github.com/edf-hpc/verrou.git verrou

    patch -p1 <verrou/valgrind.diff


### Configure and build

First, install all required dependencies (the names of relevant Debian packages
are put in parentheses as examples):

- C & C++ compilers (`build-essential`),
- autoconf & automake (`automake`),
- Python 3 (`python3`)
- C standard library with debugging symbols (`libc6-dbg`).

<p>&nbsp;</p>

Configure valgrind:

    ./autogen.sh
    ./configure --enable-only64bit --prefix=PREFIX

It is recommended to add the `--enable-verrou-fma` flag to the configuration
above if your system supports FMA instructions. Depending on your system, it may
be required to set `CFLAGS` so that it enables the use of FMA in your compiler:

    ./configure --enable-only64bit --enable-verrou-fma --prefix=PREFIX CFLAGS="-mfma"

<p>&nbsp;</p>

Build and install:

    make
    make install


### Load the environment

In order to actually use Verrou, you must load the correct environment. This can
be done using:

    source PREFIX/env.sh


### Test (optional)

#### General tests

You can test the whole platform:

    make check
    perl tests/vg_regtest --all
    
or only verrou:

    make -C tests check
    make -C verrou check
    perl tests/vg_regtest verrou
    
    
#### Specific tests

These tests are more closely related to the arithmetic part in Verrou:

    make -C verrou/unitTest


## Documentation

The documentation for verrou is available as a
[chapter in valgrind's manual](//edf-hpc.github.com/verrou/vr-manual.html).

<p>&nbsp;</p>

You can also re-build it:

    make -C docs html-docs man-pages

and browse it locally:

    iceweasel docs/html/vr-manual.html


Beware, this requires lots of tools which are not necessarily tested for in
`configure`, including (but not necessarily limited to):

  - xsltproc
  - docbook-xsl

