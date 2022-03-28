# Verrou

[![Build Status](https://travis-ci.org/edf-hpc/verrou.svg?branch=master)](https://travis-ci.org/edf-hpc/verrou) 
[![Documentation](https://img.shields.io/badge/docs-latest-blue.svg)](http://edf-hpc.github.io/verrou/vr-manual.html)

Verrou helps you look for floating-point round-off errors in programs. It
implements various forms of arithmetic, including:

- all IEEE-754 standard rounding modes;

- three variants of stochastic floating-point arithmetic based on random rounding:
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


## Installation

### Get the sources

The preferred way to get Verrou sources is to download the latest *stable*
version: [v2.2.0](https://github.com/edf-hpc/verrou/releases/latest).
Older versions are available in the [releases](https://github.com/edf-hpc/verrou/releases)
page. After downloading one of the released versions, skip to the "Configure
and build" section below.

<p>&nbsp;</p>

In order to build the *development* version of Verrou, it is necesary to first
download a specific Valgrind version, and patch it. Fetch valgrind's sources:

    git clone --branch=VALGRIND_3_17_0 --single-branch git://sourceware.org/git/valgrind.git valgrind-3.17.0+verrou-dev

or if you have proxy problem with git:// protocol:

    export https_proxy=ADDRESS_OF_PROXY
    wget https://sourceware.org/pub/valgrind/valgrind-3.17.0.tar.bz2
    tar xvfj valgrind-3.17.0.tar.bz2
    mv valgrind-3.17.0 valgrind-3.17.0+verrou-dev


Add verrou's sources to it:

    cd valgrind-3.17.0+verrou-dev
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
    ./configure --enable-only64bit --enable-verrou-fma --prefix=PREFIX

As stated above, it is recommended to use the `--enable-verrou-fma` flag if your
system supports FMA (Fused Multiply-Add) instructions. Depending on your system,
it may be required to set `CFLAGS` in order to enable the use of FMA in your
compiler:

    ./configure --enable-only64bit --enable-verrou-fma --prefix=PREFIX CFLAGS="-mfma"

Systems that don't support FMA instructions can drop the `--enable-verrou-fma`
configure switch, but be aware that this causes some tests to fail:

    ./configure --enable-only64bit --prefix=PREFIX

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
[chapter in valgrind's manual](//edf-hpc.github.io/verrou/vr-manual.html).

<p>&nbsp;</p>

You can also re-build it:

    make -C docs html-docs man-pages

and browse it locally:

    firefox docs/html/vr-manual.html


Beware, this requires lots of tools which are not necessarily tested for in
`configure`, including (but not necessarily limited to):

  - xsltproc
  - docbook-xsl


## Bibliography & References

The following papers explain in more details the internals of Verrou, as well as
some of its applications. If you use Verrou for a research work, please consider
citing one of these references:

1. François Févotte and Bruno Lathuilière. Debugging and optimization of HPC
   programs with the Verrou tool. In *International Workshop on Software
   Correctness for HPC Applications (Correctness)*, Denver, CO, USA,
   Nov. 2019. [DOI: 10.1109/Correctness49594.2019.00006](http://dx.doi.org/10.1109/Correctness49594.2019.00006)
1. Hadrien Grasland, François Févotte, Bruno Lathuilière, and David
   Chamont. Floating-point profiling of ACTS using Verrou. *EPJ Web Conf.*, 214, 2019.
   [DOI: 10.1051/epjconf/201921405025](http://dx.doi.org/10.1051/epjconf/201921405025)
1. François Févotte and Bruno Lathuilière. Studying the numerical quality of an
   industrial computing code: A case study on code_aster. In *10th International
   Workshop on Numerical Software Verification (NSV)*, pages 61--80, Heidelberg,
   Germany,
   July 2017. [DOI: 10.1007/978-3-319-63501-9_5](http://dx.doi.org/10.1007/978-3-319-63501-9_5)
1. François Févotte and Bruno Lathuilière. VERROU: a CESTAC evaluation without
   recompilation. In *International Symposium on Scientific Computing, Computer
   Arithmetics and Verified Numerics (SCAN)*, Uppsala, Sweden, September 2016.

(These references are also available in [bibtex format](verrou.bib))
