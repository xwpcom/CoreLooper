#!/bin/bash

# Assume we're in wolfssl/Docker
WOLFSSL_DIR=$(builtin cd ${BASH_SOURCE%/*}/..; pwd)

DOCKER_BUILD_OPTIONS="$1"
if [ "${DOCKER_BASE_IMAGE}" != "" ]; then
    DOCKER_BUILD_OPTIONS+=" --build-arg DOCKER_BASE_IMAGE=${DOCKER_BASE_IMAGE}"
fi

NUM_FAILURES=0

CUR_DATE=$(date -u +%F)
echo "Building wolfssl/wolfssl-builder:${CUR_DATE} as ${DOCKER_BUILD_OPTIONS}"
docker build -t wolfssl/wolfssl-builder:${CUR_DATE} ${DOCKER_BUILD_OPTIONS} "${WOLFSSL_DIR}/Docker" && \
    docker tag wolfssl/wolfssl-builder:${CUR_DATE} wolfssl/wolfssl-builder:latest && \
    docker build --build-arg DOCKER_BASE_IMAGE=wolfssl/wolfssl-builder:${CUR_DATE} -t wolfssl/testing-cross-compiler:${CUR_DATE} "${WOLFSSL_DIR}/Docker" -f Dockerfile.cross-compiler && \
    docker tag wolfssl/testing-cross-compiler:${CUR_DATE} wolfssl/testing-cross-compiler:latest

if [ $? -eq 0 ]; then
    echo "Pushing containers to DockerHub"
    docker push wolfssl/wolfssl-builder:${CUR_DATE} && docker push wolfssl/wolfssl-builder:latest && \
        docker push wolfssl/testing-cross-compiler:${CUR_DATE} && docker push wolfssl/testing-cross-compiler:latest
else
    echo "Warning: Build wolfssl/wolfssl-builder failed. Continuing"
    ((NUM_FAILURES++))
fi

echo "Building wolfssl/wolfCLU:${CUR_DATE}"
docker buildx build --pull --push --build-arg DUMMY=${CUR_DATE} -t wolfssl/wolfclu:${CUR_DATE} --platform=linux/amd64,linux/arm64,linux/arm/v7 "${WOLFSSL_DIR}/Docker/wolfCLU" && \
docker buildx build --pull --push --build-arg DUMMY=${CUR_DATE} -t wolfssl/wolfclu:latest      --platform=linux/amd64,linux/arm64,linux/arm/v7 "${WOLFSSL_DIR}/Docker/wolfCLU"
if [ $? -ne 0 ]; then
    echo "Warning: Build wolfssl/wolfclu failed. Continuing"
    ((NUM_FAILURES++))
fi

echo "Script completed in $SECONDS seconds. Had $NUM_FAILURES failures."
