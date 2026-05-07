#!/bin/bash
set -e

# Clean any existing build
rm -rf build CMakeCache.txt

# Build project
cmake . -B build
make -C build

# Run tests
cd build && ctest --output-on-failure
