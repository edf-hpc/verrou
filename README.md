# Installation

## Get the sources

Fetch valgrind's sources:

    svn co svn://svn.valgrind.org/valgrind/trunk valgrind


Add verrou's sources to it:

    cd valgrind
    git clone /netdata/H55056/git/verrou.git verrou

    patch -p0 <verrou/valgrind.diff


## Configure and build

Configure valgrind:

    ./autogen.sh
    ./configure --enable-only64bit

Build and install:

    make
    make install

## Test

Build the tests:

    make check

... and run them:

    # run all tests:
    perl tests/vg_regtest --all

    # or test only verrou:
    perl tests/vg_regtest verrou


# Documentation

Build the documentation and browse it:

    cd docs; make html-docs
    iceweasel html/vr-manual.html
