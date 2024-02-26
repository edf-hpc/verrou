# Build stage
ARG IMAGE_BASE=ubuntu:20.04

ARG VALGRIND_VERSION=3.21.0
ARG VALGRIND_BRANCH=VALGRIND_3_21_0
ARG VERROU_BRANCH=master
ARG NUM_THREADS=6

#Build Stage
FROM ${IMAGE_BASE} AS build

ARG VALGRIND_VERSION
ARG VALGRIND_BRANCH
ARG VERROU_BRANCH
ARG NUM_THREADS

RUN apt-get update \
    &&  DEBIAN_FRONTEND=noninteractive apt-get install -y --reinstall ca-certificates \
    && update-ca-certificates \
    &&  DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends git build-essential automake python3 libc6-dbg \
    && rm -fr /var/lib/apt/lists/*

RUN mkdir -p /build
WORKDIR /build

RUN git clone --branch=${VALGRIND_BRANCH} --single-branch --depth=1 git://sourceware.org/git/valgrind.git valgrind-${VALGRIND_VERSION}+verrou-dev

WORKDIR /build/valgrind-${VALGRIND_VERSION}+verrou-dev

RUN git clone --branch=${VERROU_BRANCH} --single-branch --depth=1 https://github.com/edf-hpc/verrou.git verrou \
    && cat verrou/valgrind.*diff | patch -p1

RUN  ./autogen.sh \
    && ./configure --enable-only64bit --enable-verrou-fma --prefix=/opt/valgrind-${VALGRIND_VERSION}+verrou-dev \
    && make -j ${NUM_THREADS}\
    && make install


# Final stage
FROM ${IMAGE_BASE} AS common
ARG VALGRIND_VERSION

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends python3 libc6-dbg python3-numpy  python3-matplotlib texlive-latex-extra texlive-fonts-recommended dvipng cm-super\
    && rm -fr /var/lib/apt/lists/*

COPY --from=build /opt/valgrind-${VALGRIND_VERSION}+verrou-dev /opt/valgrind-${VALGRIND_VERSION}+verrou-dev

RUN cp /opt/valgrind-${VALGRIND_VERSION}+verrou-dev/env.sh /etc/profile.d/99-valgrind+verrou.sh \
    && chmod 644 /etc/profile.d/99-valgrind+verrou.sh


# Test stage
FROM common AS test
ARG VALGRIND_VERSION

#USER root
RUN apt-get -y update \
    && DEBIAN_FRONTEND=noninteractive apt-get install --no-install-recommends -y build-essential automake \
    && rm -fr /var/lib/apt/lists/*

COPY --from=build /build/valgrind-${VALGRIND_VERSION}+verrou-dev  /build/valgrind-${VALGRIND_VERSION}+verrou-dev 

WORKDIR /build/valgrind-${VALGRIND_VERSION}+verrou-dev/verrou
RUN make -C unitTest
RUN make -C unitTest valgrind-test || cat tests/*.diff noExistingFile

#Default
FROM common

RUN useradd user -d /home/user -s /bin/bash \
    && mkdir /home/user && chown -R user:user /home/user

WORKDIR /home/user
USER user

RUN echo "Verrou image successfully built"
