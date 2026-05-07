#!/bin/bash


set -e

echo "Building Movie Recommendation Benchmark..."
mkdir -p build
cd build
cmake ..
make

echo "Running tests..."
ctest --verbose

echo "All tests completed!"