FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy source files
COPY . .

# Build the project
RUN mkdir -p build && cd build && \
    cmake .. && \
    make

# Run tests
CMD ["bash", "run_tests.sh"]