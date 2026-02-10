#!/bin/bash

set -e

mkdir -p build

readonly LIBRARY_PATH="/home/duckyshine04/Documents/libraries/"

export FASTNOISE2_PREFIX="${LIBRARY_PATH}/FastNoise2"

cmake --preset default
cmake --build build

rm -rf .cache/logs/*
