#!/bin/bash


set -e

# Build the project
cmake -B build -S .
cmake --build build

# Run tests
cd build
ctest --output-on-failure
cd ..

echo "All tests completed successfully!"
