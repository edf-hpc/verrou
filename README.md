# Installation

## Get the sources

Fetch valgrind's sources:

    svn co svn://svn.valgrind.org/valgrind/trunk valgrind


Add verrou's sources to it:

    cd valgrind
    git clone /netdata/H55056/git/verrou.git verrou

    patch -p1 <verrou/valgrind.diff


## Configure and build

Configure valgrind:

    ./autogen.sh
    ./configure --enable-only64bit --prefix=PREFIX

Build and install:

    make
    make install


## (optionally) Test

Build the tests:

    make check

... and run them:

    # run all tests:
    perl tests/vg_regtest --all

    # or test only verrou:
    perl tests/vg_regtest verrou


# Documentation

## Build the documentation

Build the documentation

    make -C docs html-docs

This requires lots of tools which are not necessarily tested for in
`configure`, including:
    - xsltproc
    - docbook-xsl

## Read it

Browse the documentation, either in place:

    iceweasel docs/html/vr-manual.html

or in the installation path:

    make install
    iceweasel PREFIX/share/doc/valgrind/html/vr-manual.html
