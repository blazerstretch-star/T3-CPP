#!/bin/bash

set -e

# Clean and rebuild to ensure fresh compilation
rm -rf build
mkdir -p build
cd build

echo "Building project..."
cmake ..
make

echo "Running all tests..."
ctest --output-on-failure

echo ""
echo "Test Summary:"
ctest --output-on-failure 2>&1 | grep -E "tests passed|tests failed" || true

exit 0
