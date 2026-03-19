#!/bin/bash

declare -A preset_set

preset_set[release]=1
preset_set[debug]=1
preset_set[profile]=1

set -e

PRESET="${1:-release}"

if [[ -z ${preset_set[$PRESET]} ]]; then
    echo "ERROR: usage is ./scripts/build.sh <preset> (default: release, debug, profile)"
    exit 1
fi

mkdir -p "build/${PRESET}"

readonly LIBRARY_PATH="/home/duckyshine04/Documents/libraries/"

export FASTNOISE2_PREFIX="${LIBRARY_PATH}/FastNoise2"

echo "INFO: Building '${PRESET}'"

cmake --preset "${PRESET}"
cmake --build "build/${PRESET}"

echo "INFO: Running '${PRESET}'"

if [[ $PRESET == "release" ]]; then
    ./build/release/duck-craft
elif [[ $PRESET == "debug" ]]; then
    ./build/debug/duck-craft
elif [[ $PRESET == "profile" ]]; then
    perf record --latency -g -F 999 ./build/profile/duck-craft
    perf report --latency
fi

rm -rf .cache/logs/*
