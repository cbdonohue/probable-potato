# SwarmApp Docker Setup

This directory contains Docker configurations for running SwarmApp in containerized environments.

## Services Overview

### Core Services
- **core**: ZeroMQ message bus and core functionality
- **http-server**: Legacy HTTP server (port 8080)
- **health-monitor**: Health monitoring service (port 8081)
- **api**: Modern Oat++-based API server (port 8083)

## Quick Start

### Development Environment
```bash
# Build and run all services
docker-compose up --build

# Run only the API service
docker-compose up api

# Run in detached mode
docker-compose up -d
```

### Production Swarm Mode
```bash
# Deploy to Docker Swarm
docker stack deploy -c docker-compose.swarm.yml swarmapp

# Scale API service
docker service scale swarmapp_api=3

# View services
docker service ls
```

## API Endpoints

### Health Check
```bash
curl http://localhost:8083/health
# Response: {"status":"healthy","timestamp":"2024-01-01T00:00:00Z","version":"1.0.0","hostname":"swarm-app"}
```

### Server Status
```bash
curl http://localhost:8083/status
# Response: {"status":"running","uptime":"0s","requests_processed":0,"active_connections":0,"version":"1.0.0"}
```

### API Information
```bash
curl http://localhost:8083/api/info
# Response: {"name":"SwarmApp API","version":"1.0.0","description":"Distributed, modular C++ application framework API","documentation_url":"/api/docs"}
```

### Welcome Message
```bash
curl http://localhost:8083/
# Response: {"name":"SwarmApp","version":"1.0.0","description":"Welcome to SwarmApp API","documentation_url":"/api/info"}
```

## Environment Variables

### API Service
- `API_HOST`: Server host (default: 0.0.0.0)
- `API_PORT`: Server port (default: 8083)
- `API_MAX_CONNECTIONS`: Maximum connections (default: 100)
- `API_ENABLE_CORS`: Enable CORS (default: true)

### ZeroMQ Configuration
- `ZMQ_PUB_PORT`: Publisher port (default: 5555)
- `ZMQ_SUB_PORT`: Subscriber port (default: 5556)

## Building Images

### Build API Image Only
```bash
# From docker directory
./build-api.sh

# Or manually from docker directory
docker build -f Dockerfile.api -t swarm-api:latest ..
```

### Test API Build
```bash
# From docker directory
./test-build.sh
```

### Build All Images
```bash
# Build main application
docker build -t swarm-app:latest .

# Build individual services
docker build -f Dockerfile.api -t swarm-api:latest ..
```

## Health Checks

All services include health checks:
- **API**: `curl -f http://localhost:8083/health`
- **HTTP Server**: `curl -f http://localhost:8080/health`
- **Health Monitor**: Internal health monitoring

## Networking

### Development
- Uses `bridge` network driver
- Services communicate via localhost

### Production
- Uses `overlay` network driver
- Services communicate across swarm nodes

## Monitoring

### View Logs
```bash
# All services
docker-compose logs

# API service only
docker-compose logs api

# Follow logs
docker-compose logs -f api
```

### Service Status
```bash
# Docker Compose
docker-compose ps

# Docker Swarm
docker service ls
docker service ps swarmapp_api
```

## Troubleshooting

### Port Conflicts
If ports are already in use, modify the port mappings in the compose files:
```yaml
ports:
  - "8084:8083"  # Map host port 8084 to container port 8083
```

### Build Issues
If the build fails due to Oat++ dependencies:
```bash
# Clean and rebuild
docker-compose down
docker-compose build --no-cache
docker-compose up
```

### Health Check Failures
If health checks are failing:
```bash
# Check service logs
docker-compose logs api

# Test endpoint manually
curl http://localhost:8083/health
```

## Architecture

```
┌─────────────┐    ┌─────────────┐    ┌─────────────┐
│   Core      │    │ HTTP Server │    │     API     │
│  (5555/6)   │    │   (8080)    │    │   (8083)    │
└─────────────┘    └─────────────┘    └─────────────┘
       │                   │                   │
       └───────────────────┼───────────────────┘
                           │
                    ┌─────────────┐
                    │Health Monitor│
                    │   (8081)    │
                    └─────────────┘
```

## Security Considerations

- All services run as non-root users
- Health checks verify service availability
- CORS can be disabled via environment variables
- Network isolation via Docker networks
