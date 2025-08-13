cmake_minimum_required(VERSION 3.10)
project(SwarmApp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(include)

# Main executable
add_executable(swarm-app src/main.cpp)

# Link libraries
target_link_libraries(swarm-app Threads::Threads)

# Tests
enable_testing()
add_executable(test-swarm-app tests/test_main.cpp)
target_link_libraries(test-swarm-app Threads::Threads)

# Add test
add_test(NAME UnitTests COMMAND test-swarm-app)

# Installation
install(TARGETS swarm-app
        RUNTIME DESTINATION bin)

# Set version info
set_target_properties(swarm-app PROPERTIES
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME swarm-app
)
