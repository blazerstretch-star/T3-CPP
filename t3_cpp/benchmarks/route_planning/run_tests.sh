#!/bin/bash

# Build and run tests for route_planning benchmark


set -e

echo "Building route_planning benchmark..."
mkdir -p build
cd build

cmake ..
make

echo "Running tests..."
ctest --verbose

echo "All tests completed!"