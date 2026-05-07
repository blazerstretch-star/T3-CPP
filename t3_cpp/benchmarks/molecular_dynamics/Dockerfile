FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

# Install build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy source files
COPY src/ /workspace/src/
COPY tests/ /workspace/tests/
COPY CMakeLists.txt /workspace/

# Build
RUN mkdir -p build && cd build && \
    cmake .. && \
    make

# Run tests by default
CMD ["ctest", "--test-dir", "build", "--output-on-failure"]
