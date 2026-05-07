#!/bin/bash

# Build and run tests for SPH benchmark

set -e

# Clean all build artifacts and CMake cache
rm -rf build build_test CMakeCache.txt CMakeFiles
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build
make

# Run tests
ctest --output-on-failure

echo "All tests completed!"
