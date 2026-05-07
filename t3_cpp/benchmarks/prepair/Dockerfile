FROM ubuntu:22.04

# Install CGAL, GDAL and build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    clang \
    git \
    pkg-config \
    libcgal-dev \
    libgdal-dev \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /workspace

# Copy repository
COPY . .

# Make run_tests.sh executable
RUN chmod +x run_tests.sh

# Default command
CMD ["./run_tests.sh"]
