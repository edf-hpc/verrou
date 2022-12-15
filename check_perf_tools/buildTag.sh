#!/bin/bash
CURRENT_REP=$PWD
BUILD_REP=$1
INSTALL_REP=${CURRENT_REP}/$1/install
VALGRIND_ARCHIVE=$2
FLAGS=$3





echo "BUILD_REP:" ${BUILD_REP}
echo "VALGRIND_ARCHIVE:" ${VALGRIND_ARCHIVE}
echo "VALGRIND_NAME: "${VALGRIND_NAME}
echo "FLAGS:" ${FLAGS}


mkdir ${BUILD_REP}
tar xvfz ${VALGRIND_ARCHIVE} --directory ${BUILD_REP}

VALGRIND_NAME=`ls ${CURRENT_REP}/${BUILD_REP}`
VALGRIND_PATH=${CURRENT_REP}/${BUILD_REP}/${VALGRIND_NAME}

echo "VALGRIND_PATH:" ${VALGRIND_PATH}

cd ${VALGRIND_PATH} || exit 42

./autogen.sh
./configure --enable-only64bit ${FLAGS} --prefix=${INSTALL_REP}

make -j 6
make install









