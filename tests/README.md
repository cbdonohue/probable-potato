# SwarmApp Test Suite

This directory contains comprehensive unit tests for the SwarmApp standalone applications and swarm system.

## Test Structure

The test suite is organized into several test files, each focusing on different aspects of the system:

### 1. Core Tests (`test_main.cpp`)
- **Purpose**: Tests the core functionality of the SwarmApp system
- **Coverage**: 
  - MessageBus basic and async functionality
  - Multiple subscribers and message routing
  - MessageBus statistics and lifecycle
  - Error handling and exception safety
  - Module base class functionality
  - ModuleManager basic operations
  - ZeroMQ integration

### 2. ZeroMQ Message Bus Tests (`test_zeromq_message_bus.cpp`)
- **Purpose**: Tests the ZeroMQ message bus implementation
- **Coverage**:
  - ZeroMQ publisher/subscriber patterns
  - Message serialization/deserialization
  - Network communication
  - Message routing and filtering

### 3. Standalone Applications Tests (`test_standalone_apps.cpp`)
- **Purpose**: Tests each standalone application individually and in integration
- **Coverage**:
  - Health Monitor standalone application
  - HTTP Server standalone application
  - Core Service standalone application
  - Monolithic standalone application (main.cpp)
  - Standalone app integration
  - Error handling and edge cases
  - Performance and concurrency
  - Resource usage

### 4. Individual Standalone Tests (`test_individual_standalone.cpp`)
- **Purpose**: Detailed testing of individual standalone applications
- **Coverage**:
  - Detailed health monitor functionality
  - HTTP server configuration and endpoints
  - Core service module management
  - Monolithic application setup
  - Edge cases and error conditions
  - Performance and resource usage
  - Configuration validation

### 5. Swarm Integration Tests (`test_swarm_integration.cpp`)
- **Purpose**: Tests the complete swarm system and distributed functionality
- **Coverage**:
  - Complete swarm system integration
  - Multi-node swarm communication
  - Load balancing functionality
  - Auto-scaling capabilities
  - Fault tolerance and recovery
  - Performance under load
  - Security features

## Prerequisites

Before running the tests, ensure you have the following dependencies installed:

### Required Dependencies
- **CMake** (version 3.10 or higher)
- **GCC/G++** (C++17 compatible)
- **Make**
- **Google Test** (libgtest-dev)
- **ZeroMQ** (libzmq3-dev)

### Optional Dependencies
- **libcurl** (libcurl4-openssl-dev) - For HTTP testing

### Installing Dependencies on Ubuntu/Debian
```bash
sudo apt update
sudo apt install cmake build-essential libgtest-dev libzmq3-dev libcurl4-openssl-dev
```

### Installing Dependencies on CentOS/RHEL
```bash
sudo yum install cmake gcc-c++ gtest-devel zeromq-devel libcurl-devel
```

## Building and Running Tests

### Using the Test Runner Script (Recommended)

The easiest way to run tests is using the provided test runner script:

```bash
# Run all tests
./scripts/run_tests.sh

# Clean build and run all tests
./scripts/run_tests.sh -c

# Run only standalone applications tests
./scripts/run_tests.sh standalone

# Run only swarm integration tests
./scripts/run_tests.sh swarm

# Build only (don't run tests)
./scripts/run_tests.sh -b

# Run tests only (assumes build exists)
./scripts/run_tests.sh -t

# Show help
./scripts/run_tests.sh -h
```

### Manual Build and Test

If you prefer to build and run tests manually:

```bash
# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build the project
make -j$(nproc)

# Run all tests
ctest --output-on-failure --verbose

# Run specific test suites
./test-swarm-app                    # Core tests
./test-zeromq-message-bus           # ZeroMQ tests
./test-standalone-apps              # Standalone apps tests
./test-individual-standalone        # Individual standalone tests
./test-swarm-integration            # Swarm integration tests
```

## Test Output

### Test Results
Tests generate XML output files in the `build/` directory:
- `test-results-core.xml` - Core unit test results
- `test-results-zeromq.xml` - ZeroMQ message bus test results
- `test-results-standalone.xml` - Standalone applications test results
- `test-results-individual.xml` - Individual standalone test results
- `test-results-swarm.xml` - Swarm integration test results

### Test Coverage
The test suite provides comprehensive coverage of:

#### Standalone Applications
- **Health Monitor**: Configuration, health checks, status monitoring, error handling
- **HTTP Server**: Server startup/shutdown, endpoint testing, configuration validation
- **Core Service**: Module management, message bus, lifecycle management
- **Monolithic App**: Complete application integration, inter-module communication

#### Swarm System
- **Multi-node Communication**: Inter-node message passing, status synchronization
- **Load Balancing**: Request distribution, health monitoring, failover
- **Auto-scaling**: Dynamic scaling based on metrics, instance management
- **Fault Tolerance**: Failure detection, recovery mechanisms, replication
- **Performance**: High-throughput testing, resource usage monitoring
- **Security**: Authentication, rate limiting, intrusion detection

#### Edge Cases and Error Handling
- Invalid configurations
- Network failures
- Resource exhaustion
- Concurrent access
- Exception safety

## Test Categories

### Unit Tests
- Individual component testing
- Isolated functionality verification
- Mock object usage
- Fast execution

### Integration Tests
- Component interaction testing
- End-to-end functionality
- Real HTTP requests (when possible)
- Network communication

### Performance Tests
- Load testing
- Resource usage monitoring
- Scalability verification
- Response time measurement

### Security Tests
- Input validation
- Authentication testing
- Rate limiting verification
- Security event handling

## Continuous Integration

The test suite is designed to work with CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
name: Tests
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: |
          sudo apt update
          sudo apt install cmake build-essential libgtest-dev libzmq3-dev libcurl4-openssl-dev
      - name: Run tests
        run: ./scripts/run_tests.sh -c
```

## Troubleshooting

### Common Issues

#### Build Failures
- **Missing dependencies**: Install required packages
- **CMake version**: Ensure CMake 3.10+ is installed
- **Compiler version**: Ensure C++17 compatible compiler

#### Test Failures
- **HTTP tests failing**: This is expected if no HTTP server is running
- **Port conflicts**: Tests use various ports (8081-8110), ensure they're available
- **Permission issues**: Ensure test runner script is executable

#### Performance Issues
- **Slow test execution**: Tests include realistic timeouts and delays
- **Resource usage**: Some tests create multiple threads and processes

### Debugging Tests

To debug failing tests:

```bash
# Run specific test with verbose output
./test-standalone-apps --gtest_filter=StandaloneAppsTest.HealthMonitorStandalone --gtest_verbose

# Run with debugger
gdb --args ./test-swarm-integration --gtest_filter=SwarmIntegrationTest.CompleteSwarmSystem

# Run with valgrind for memory checking
valgrind --leak-check=full ./test-individual-standalone
```

## Contributing

When adding new tests:

1. **Follow naming conventions**: Use descriptive test names
2. **Add to appropriate test file**: Choose the most relevant test file
3. **Update CMakeLists.txt**: Add new test executables if needed
4. **Document test purpose**: Add comments explaining what the test verifies
5. **Handle cleanup**: Ensure tests clean up after themselves
6. **Consider performance**: Keep tests fast and efficient

## Test Maintenance

### Regular Tasks
- Update test dependencies
- Review test coverage
- Optimize slow tests
- Update test documentation

### Test Data
- Use realistic test data
- Avoid hardcoded values
- Make tests deterministic
- Handle test environment variations

## Support

For issues with the test suite:
1. Check the troubleshooting section
2. Review test output and logs
3. Verify dependencies are correctly installed
4. Check for port conflicts
5. Review recent changes to the codebase
