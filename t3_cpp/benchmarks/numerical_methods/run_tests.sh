#!/bin/bash

set -e

cd /workspace
mkdir -p build
cd build
cmake ..
make -j$(nproc)
ctest --output-on-failure
