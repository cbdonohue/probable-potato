FROM gcc:11 AS builder

WORKDIR /app

# Install CMake, ZeroMQ, and other development libraries
RUN apt-get update && apt-get install -y \
    cmake \
    libzmq3-dev \
    pkg-config \
    git \
    libboost-all-dev \
    libcurl4-openssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Oat++ framework
RUN git clone https://github.com/oatpp/oatpp.git && \
    cd oatpp && \
    mkdir -p build && cd build && \
    cmake .. && \
    make -j$(nproc) && \
    make install && \
    cd ../.. && \
    rm -rf oatpp

COPY .. .
RUN rm -rf build && mkdir build && cd build && cmake .. && make

FROM debian:bullseye-slim

RUN apt-get update && apt-get install -y \
    ca-certificates \
    curl \
    libzmq5 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/swarm-app .

EXPOSE 8080 8083

CMD ["./swarm-app"]
