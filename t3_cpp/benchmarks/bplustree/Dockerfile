FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

COPY src/ /workspace/src/
COPY tests/ /workspace/tests/
COPY CMakeLists.txt /workspace/
COPY run_tests.sh /workspace/

RUN chmod +x run_tests.sh

CMD ["./run_tests.sh"]
