FROM gcc:11 as builder

WORKDIR /app

COPY . .
RUN make clean && make

FROM debian:bullseye-slim

RUN apt-get update && apt-get install -y \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /app/swarm-app .

EXPOSE 5000

CMD ["./swarm-app"]
