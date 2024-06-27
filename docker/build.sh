#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

TARGET="app"
IMAGE="caiwan/grafkit-vk"
TAG="latest"
BASE_IMAGE="ubuntu:20.04"

case $1 in
    app)
        TARGET="app"
        TAG="latest"
        ;;
    dev)
        TARGET="dev"
        TAG="latest-dev"
        ;;
    *)
        echo "Invalid target: $1"
        exit 1
        ;;
esac

mkdir -p ${SCRIPT_DIR}/context

DOCKER_BUILDKIT=1 docker build \
--target $TARGET \
--build-arg BASE_IMAGE=$BASE_IMAGE \
-t "$IMAGE:$TAG" \
-f ${SCRIPT_DIR}/Dockerfile ${SCRIPT_DIR}/context

# TODO remove only if needed eg. clean build
# rm -rf ./docker/context
