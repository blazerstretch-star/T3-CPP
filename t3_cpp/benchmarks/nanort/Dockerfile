FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

# Build Google Test
RUN cd /usr/src/gtest && \
    cmake . && \
    make && \
    cp lib/*.a /usr/lib/ || cp *.a /usr/lib/

WORKDIR /workspace

COPY . /workspace/

RUN mkdir -p build && cd build && \
    cmake .. && \
    make

CMD ["./build/nanort_tests"]
