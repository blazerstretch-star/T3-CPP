#!/bin/bash

set -e

cd /workspace
mkdir -p build
cd build
cmake ..
make
ctest --output-on-failure
