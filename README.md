# SwarmApp

A distributed, modular C++ application framework with health monitoring, HTTP server capabilities, and swarm orchestration features.

## Features

- **Modular Architecture**: Plugin-based module system with dynamic loading
- **Health Monitoring**: Real-time health checks and status monitoring
- **API Server**: Modern REST API using Oat++ framework
- **Message Bus**: ZeroMQ-based inter-module communication
- **Swarm Support**: Multi-node deployment and orchestration
- **Comprehensive Testing**: Full unit and integration test suite

## Dependencies

Before building SwarmApp, you need to install the following dependencies:

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config \
    libzmq3-dev \
    libcurl4-openssl-dev \
    libgtest-dev \
    libgmock-dev
```

### CentOS/RHEL/Fedora
```bash
sudo yum update -y
sudo yum install -y \
    gcc-c++ \
    cmake \
    pkg-config \
    zeromq-devel \
    libcurl-devel \
    gtest-devel \
    gmock-devel
```

### macOS
```bash
brew install \
    cmake \
    pkg-config \
    zeromq \
    curl \
    googletest
```

### Automated Installation
You can also use the provided script:
```bash
./scripts/install_dependencies.sh
```

## Building

```bash
mkdir -p build
cd build
cmake ..
make
```

## Testing

```bash
# Run all tests
make test

# Run specific test suites
./test-standalone-apps
./test-individual-standalone
./test-swarm-integration

# Run with verbose output
ctest --output-on-failure --verbose
```

## Usage

### Standalone Applications

#### Health Monitor
```bash
./swarm-health-monitor --config health_monitor.conf
```

#### API Server (Oat++)
```bash
./swarm-api --config api.conf
# or with command line options:
./api-standalone --host 0.0.0.0 --port 8080 --max-connections 100
```

#### Core Service
```bash
./swarm-core --config core.conf
```

### Monolithic Application
```bash
./swarm-app --config swarm.conf
```

## Configuration

Configuration files use a simple key-value format:

```ini
# API Server Configuration (Oat++)
port=8080
host=127.0.0.1
max_connections=100
enable_cors=true

# Health Monitor Configuration
default_timeout_ms=5000
default_interval_ms=10000
max_failures=3
enable_notifications=true
```

## Architecture

SwarmApp consists of several core components:

- **Module Manager**: Handles module lifecycle and dependencies
- **Message Bus**: ZeroMQ-based communication system
- **Health Monitor**: Service health checking and monitoring
- **API Server**: Modern REST API with JSON DTOs
- **Swarm Orchestrator**: Multi-node deployment management

## Development

### Project Structure
```
swarm/
├── src/                    # Source code
│   ├── core/              # Core module management
│   ├── health_monitor/    # Health monitoring
│   └── standalone/        # Standalone applications
├── tests/                 # Test suite
├── scripts/               # Build and utility scripts
├── docker/                # Docker configuration
└── docs/                  # Documentation
```

### Adding New Modules

1. Create a new module class inheriting from `Module`
2. Implement required virtual methods
3. Register the module with the ModuleManager
4. Add configuration options
5. Write unit tests

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.
