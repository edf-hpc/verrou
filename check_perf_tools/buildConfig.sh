#!/bin/bash
CURRENT_REP=$PWD
BUILD_REP=$1
INSTALL_REP=${CURRENT_REP}/$1/install
VALGRIND_ARCHIVE=$2
BRANCH_VERROU=$3
FLAGS=$4

VALGRIND_NAME=$(basename ${VALGRIND_ARCHIVE} |sed "s/.tar.bz2//")



echo "BUILD_REP:" ${BUILD_REP}
echo "VALGRIND_ARCHIVE:" ${VALGRIND_ARCHIVE}
echo "BRANCH_VERROU:" ${BRANCH_VERROU}
echo "VALGRIND_NAME: "${VALGRIND_NAME}
echo "FLAGS:" ${FLAGS}


mkdir ${BUILD_REP}
tar xvfj ${VALGRIND_ARCHIVE} --directory ${BUILD_REP}

VALGRIND_PATH=${CURRENT_REP}/${BUILD_REP}/${VALGRIND_NAME}

echo "VALGRIND_PATH:" ${VALGRIND_PATH}
mkdir ${VALGRIND_PATH}/verrou
cd ..
git archive origin/${BRANCH_VERROU} | tar -x -C ${VALGRIND_PATH}/verrou ||exit 42

cd ${VALGRIND_PATH} || exit 42
patch -p1 <verrou/valgrind.diff

./autogen.sh
./configure --enable-only64bit ${FLAGS} --prefix=${INSTALL_REP}

make -j 6
make install









