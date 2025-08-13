# SwarmApp

A minimal, high-performance C++ HTTP server application designed for Docker Swarm deployment.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)](https://cmake.org/)

## Features

- 🚀 **High Performance**: Native C++ implementation
- 🧩 **Modular Architecture**: Pluggable modules that can exist individually or interact
- 🔄 **Inter-Module Communication**: Message bus for loose coupling between modules
- 🐳 **Docker Swarm Ready**: Optimized for container orchestration
- 🧪 **Well Tested**: Comprehensive unit test suite using Google Test (20 tests)
- 🔧 **Multiple Build Systems**: Support for both Make and CMake
- 📦 **Production Ready**: Multi-stage Docker builds
- 🔍 **Health Monitoring**: Built-in health monitoring system
- 🔄 **Auto Recovery**: Automatic restart on failure
- 📊 **Health Monitoring**: Built-in health check endpoint

## 🧩 Modular Architecture

SwarmApp is built with a modular architecture that allows components to exist independently while enabling seamless interaction:

### Core Components
- **Module Interface**: Base class that all modules inherit from
- **Module Manager**: Handles module lifecycle, dependencies, and configuration
- **Message Bus**: Enables inter-module communication through publish/subscribe pattern

### Available Modules
- **HTTP Server Module**: Provides RESTful API endpoints
- **Health Monitor Module**: Monitors system health and provides status reporting

### Module Benefits
- **Scalability**: Add or remove modules without affecting others
- **Maintainability**: Each module has a single responsibility
- **Testability**: Modules can be tested in isolation
- **Flexibility**: Modules can be configured independently
- **Extensibility**: Easy to add new modules

## Quick Start

### Prerequisites
- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.10+ (optional, for CMake builds)
- Docker (for containerized deployment)
- GitLab (for CI/CD pipeline)

### Building from Source

#### Using Make (Recommended)
```bash
# Build the application
make

# Run tests
make test
./test-swarm-app

# Or run tests with Make
make run-tests

# Or build and test in one command
make check

# Clean build artifacts
make clean
```

#### Using CMake
```bash
# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run tests
make test
```

### Local Development
```bash
# Build and run locally using Docker Compose
cd docker
docker-compose up --build
```

### Docker Swarm Deployment

1. **Initialize Docker Swarm** (if not already done):
```bash
docker swarm init
```

2. **Build the application image**:
```bash
cd docker
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
curl http://localhost:8080
```

## 🔄 CI/CD Pipeline

SwarmApp includes CI/CD configurations for both **GitHub Actions** and **GitLab CI/CD**.

### GitHub Actions (Recommended)

Automated build, test, and deployment using GitHub's native CI/CD.

#### Pipeline Features
- **Automated Build**: Compiles C++ application using CMake
- **Unit Testing**: Runs Google Test suite with JUnit reports
- **Docker Packaging**: Builds and pushes to GitHub Container Registry
- **Staging Deployment**: Automatic deployment to staging environment
- **Production Deployment**: Manual approval deployment to production

#### Quick Setup
1. **Repository**: Your code is already in GitHub
2. **Workflows**: Automatically triggered on pushes to `main` and `develop`
3. **Registry**: Uses GitHub Container Registry (ghcr.io)

#### Pipeline Files
- `.github/workflows/ci.yml` - Full CI/CD pipeline with deployment
- `.github/workflows/ci-simple.yml` - Simple build and test pipeline

### GitLab CI/CD (Alternative)

For teams using GitLab, the project includes GitLab CI/CD configuration.

#### Pipeline Features
- **Automated Build**: Compiles C++ application using CMake
- **Unit Testing**: Runs Google Test suite with JUnit reports
- **Docker Packaging**: Builds and pushes container images
- **Staging Deployment**: Automatic deployment to staging environment
- **Production Deployment**: Manual approval deployment to production
- **Security Scanning**: Optional security vulnerability scanning

#### Quick Setup
1. Push code to GitLab repository
2. Configure GitLab variables (if needed)
3. Pipeline runs automatically on commits and merge requests

#### Pipeline Files
- `.gitlab-ci.yml` - Full pipeline with deployment stages
- `.gitlab-ci-simple.yml` - Simple pipeline for development
- `scripts/deploy.sh` - Deployment script for Docker Swarm

For detailed GitLab CI/CD documentation, see [docs/CI_CD.md](docs/CI_CD.md).

## 🚀 Deployment

### Using the Deployment Script
```bash
# Deploy to production
./scripts/deploy.sh production swarm-app

# Deploy to staging
./scripts/deploy.sh staging swarm-app-staging

# Check deployment status
./scripts/deploy.sh status

# Perform health check
./scripts/deploy.sh health

# Rollback deployment
./scripts/deploy.sh rollback
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

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for a list of changes and releases.

## Application Endpoints

- `GET /` - Main endpoint returning hostname and version
- `GET /health` - Health check endpoint

## Project Structure

```
swarm-app/
├── src/                    # Source files
│   ├── main.cpp           # Main application entry point
│   ├── core/              # Core module system
│   │   ├── message_bus.cpp
│   │   └── module_manager.cpp
│   └── modules/           # Individual modules
│       ├── http-server/   # HTTP server module
│       │   └── http_server_module.cpp
│       └── health-monitor/ # Health monitoring module
│           └── health_monitor_module.cpp
├── include/               # Header files
│   ├── core/              # Core module interfaces
│   │   ├── module.h       # Base module interface
│   │   ├── message_bus.h  # Inter-module communication
│   │   └── module_manager.h # Module lifecycle management
│   └── modules/           # Module-specific headers
│       ├── http_server_module.h
│       └── health_monitor_module.h
├── tests/                 # Test files
│   └── test_main.cpp      # Unit test suite
├── docker/                # Docker configurations
│   ├── Dockerfile         # Container build configuration
│   ├── docker-compose.yml # Local development
│   └── docker-compose.swarm.yml # Swarm deployment
├── docs/                  # Documentation (future)
├── CMakeLists.txt        # CMake build configuration
├── Makefile              # Make build configuration
└── README.md             # This file
```
