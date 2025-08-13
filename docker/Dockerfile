FROM gcc:11 AS builder

WORKDIR /app

# Install CMake
RUN apt-get update && apt-get install -y cmake && rm -rf /var/lib/apt/lists/*

COPY . .
RUN rm -rf build && mkdir build && cd build && cmake .. && make

FROM debian:bullseye-slim

RUN apt-get update && apt-get install -y \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/build/swarm-app .

EXPOSE 8080

CMD ["./swarm-app"]
