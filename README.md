# SwarmApp

A minimal, high-performance C++ HTTP server application designed for Docker Swarm deployment.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10+-green.svg)](https://cmake.org/)

## Features

- 🚀 **High Performance**: Native C++ implementation
- 🐳 **Docker Swarm Ready**: Optimized for container orchestration
- 🧪 **Well Tested**: Comprehensive unit test suite (31 tests)
- 🔧 **Multiple Build Systems**: Support for both Make and CMake
- 📦 **Production Ready**: Multi-stage Docker builds
- 🔄 **Auto Recovery**: Automatic restart on failure
- 📊 **Health Monitoring**: Built-in health check endpoint

## Quick Start

### Prerequisites
- C++11 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.10+ (optional, for CMake builds)
- Docker (for containerized deployment)

### Building from Source

#### Using Make (Recommended)
```bash
# Build the application
make

# Run tests
make test
./test-swarm-app

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
│   └── main.cpp           # Main application entry point
├── include/               # Header files
│   └── server.h           # HTTP server class definition
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
