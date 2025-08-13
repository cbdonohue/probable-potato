# Minimal Docker Swarm Application

A simple Flask web application designed to run on Docker Swarm.

## Quick Start

### Local Development
```bash
# Build and run locally
docker-compose up --build
```

### Docker Swarm Deployment

1. **Initialize Docker Swarm** (if not already done):
```bash
docker swarm init
```

2. **Build the application image**:
```bash
docker build -t swarm-app:latest .
```

3. **Deploy to Swarm**:
```bash
docker stack deploy -c docker-compose.swarm.yml swarm-app
```

4. **Check the deployment**:
```bash
docker stack services swarm-app
docker stack ps swarm-app
```

5. **Access the application**:
```bash
curl http://localhost:5000
```

## Management Commands

- **Scale the service**:
```bash
docker service scale swarm-app_web=5
```

- **Update the service**:
```bash
docker service update swarm-app_web --image swarm-app:latest
```

- **Remove the stack**:
```bash
docker stack rm swarm-app
```

- **Leave swarm** (if needed):
```bash
docker swarm leave --force
```

## Application Endpoints

- `GET /` - Main endpoint returning hostname and version
- `GET /health` - Health check endpoint

## Features

- Simple Flask web application
- Docker Swarm ready with overlay networking
- Health check endpoint
- Rolling updates support
- Automatic restart on failure
