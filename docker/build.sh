#!/bin/bash

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

TARGET="app"
IMAGE="caiwan/grafkit-vk"
TAG="latest"
BASE_IMAGE="debian:bookworm-slim"

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

DOCKER_BUILDKIT=1 docker build \
--target $TARGET \
--build-arg BASE_IMAGE=$BASE_IMAGE \
-t "$IMAGE:$TAG" \
-f ${SCRIPT_DIR}/Dockerfile ${SCRIPT_DIR}/../

# TODO remove only if needed eg. clean build
# rm -rf ./docker/context
