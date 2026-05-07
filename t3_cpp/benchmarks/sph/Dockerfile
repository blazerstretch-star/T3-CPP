FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build essentials and dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libglm-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy source files
COPY src/ /workspace/src/
COPY tests/ /workspace/tests/
COPY CMakeLists.txt /workspace/

# Build the project (clean first to avoid cache issues)
RUN rm -rf build && mkdir -p build && cd build && \
    cmake .. && \
    make

# Run tests by default
CMD ["ctest", "--test-dir", "build", "--output-on-failure"]
