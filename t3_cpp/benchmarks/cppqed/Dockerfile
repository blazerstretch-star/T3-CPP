FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build essentials and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++-12 \
    libboost-all-dev \
    libeigen3-dev \
    && rm -rf /var/lib/apt/lists/*

# Set C++23 compiler
ENV CXX=g++-12
ENV CC=gcc-12

WORKDIR /workspace

# Copy benchmark files
COPY src/ /workspace/src/
COPY tests/ /workspace/tests/
COPY CMakeLists.txt /workspace/

# Build
RUN cmake -B build -S . && cmake --build build

# Run tests
CMD ["ctest", "--test-dir", "build", "--output-on-failure"]
