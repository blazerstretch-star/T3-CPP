#!/bin/bash


set -e

cd /workspace
cmake .
make
ctest --output-on-failure

exit $?
