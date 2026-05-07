#!/bin/bash

set -e  # Exit on error

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build all targets
make

# Run all tests
ctest --output-on-failure

# Exit with test result
exit $?
