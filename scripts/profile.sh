#!/bin/bash

perf record --latency ./build/duck-craft
perf report --latency

