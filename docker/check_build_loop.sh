#!/bin/sh

LISTOFIMAGE_ORG="ubuntu:20.04 ubuntu:latest debian:buster debian:bullseye debian:testing"

# fail : debian:stretch problem with matplotlib

VERROU_BRANCH=master

for IMAGE in $LISTOFIMAGE_ORG
do
    echo "Test build image:" $IMAGE
    VERROU_IMAGE=$(echo  $IMAGE |sed -s "s/:/-/")-verrou
    docker build .  -t ${VERROU_IMAGE} --build-arg IMAGE_BASE=$IMAGE  --build-arg VERROU_BRANCH=$VERROU_BRANCH || exit 42
done
