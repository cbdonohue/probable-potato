# SwarmApp Documentation

This directory contains the documentation for the SwarmApp project.

## Generated Documentation

The API documentation is generated using [Doxygen](https://www.doxygen.nl/). The generated HTML documentation is located in `docs/doxygen/html/` after building.

## Building Documentation

### Prerequisites

- Doxygen (version 1.8.0 or later)
- CMake (for build integration)

### Using CMake (Recommended)

```bash
# Build the project and documentation
mkdir build && cd build
cmake ..
make docs
```

The documentation will be generated in `build/docs/doxygen/html/`.

### Using Doxygen Directly

```bash
# Generate documentation
doxygen Doxyfile
```

The documentation will be generated in `docs/doxygen/html/`.

## Documentation Structure

### Main Page
- **Overview**: Introduction to the SwarmApp framework
- **Features**: Key features and capabilities
- **Architecture**: High-level architecture overview
- **Getting Started**: Quick start guide
- **Examples**: Code examples and usage patterns

### API Reference

#### Core Classes
- **Module**: Base interface for all application modules
- **ModuleManager**: Manages module lifecycle and dependencies
- **MessageBus**: Inter-module communication using ZeroMQ

#### Module Classes
- **HttpServerModule**: HTTP server for external communication
- **HealthMonitorModule**: System health monitoring

### Code Examples

The documentation includes code examples from:
- `src/main.cpp`: Main application entry point
- `src/modules/http-server/http_server_module.cpp`: HTTP server implementation
- `src/modules/health-monitor/health_monitor_module.cpp`: Health monitor implementation

## Documentation Features

- **Cross-references**: Links between related classes and methods
- **Call graphs**: Visual representation of function call relationships
- **Class diagrams**: UML-style class relationship diagrams
- **Search functionality**: Full-text search across all documentation
- **Source code integration**: Direct links to source code

## Configuration

The documentation is configured via the `Doxyfile` in the project root. Key settings include:

- **Input**: `include/`, `src/`, and `docs/mainpage.dox`
- **Output**: `docs/doxygen/`
- **HTML generation**: Enabled with search functionality
- **Graph generation**: Class diagrams and call graphs enabled
- **Source browsing**: Links to source code enabled

## Contributing

When adding new classes or methods to the codebase:

1. Add Doxygen comments to header files
2. Use `@brief`, `@param`, `@return`, and `@note` tags appropriately
3. Group related methods using `@name` and `@{`/`@}` tags
4. Add cross-references using `@see` tags
5. Document exceptions and thread safety notes

### Comment Style

```cpp
/**
 * @brief Brief description of the function
 * 
 * Detailed description of what the function does, its behavior,
 * and any important notes about usage.
 * 
 * @param param1 Description of the first parameter
 * @param param2 Description of the second parameter
 * @return Description of the return value
 * @note Important usage notes
 * @see RelatedClass
 * @exception std::runtime_error When something goes wrong
 */
```

## Viewing Documentation

After generation, open `docs/doxygen/html/index.html` in your web browser to view the documentation.

## Troubleshooting

### Doxygen Not Found
If CMake reports "Doxygen not found", install Doxygen:

**Ubuntu/Debian:**
```bash
sudo apt-get install doxygen
```

**CentOS/RHEL:**
```bash
sudo yum install doxygen
```

**macOS:**
```bash
brew install doxygen
```

### Documentation Not Updating
If changes aren't reflected in the documentation:

1. Clean the build directory: `make clean`
2. Regenerate documentation: `make docs`
3. Check that Doxygen comments are properly formatted

### Missing Classes or Methods
Ensure that:
1. All public methods have Doxygen comments
2. Header files are included in the INPUT path
3. File patterns include your file extensions
