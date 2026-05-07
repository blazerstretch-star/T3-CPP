#!/bin/bash

set -e

cd /workspace
mkdir -p build
cd build

cmake ..
make -j$(nproc)

echo "Running tests..."
ctest --output-on-failure

echo "All tests passed!"
