FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

# Source code will be mounted at runtime to avoid Docker layer caching
# This ensures every evaluation compiles fresh code

CMD ["bash", "-c", "rm -rf build && mkdir -p build && cd build && cmake .. && make && ctest --output-on-failure"]
