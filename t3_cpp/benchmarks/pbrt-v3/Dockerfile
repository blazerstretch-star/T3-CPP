FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

# Build Google Test
WORKDIR /usr/src/gtest
RUN cmake . && make && cp lib/*.a /usr/lib/

WORKDIR /workspace
COPY . .

CMD ["/bin/bash"]
