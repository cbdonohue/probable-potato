cmake_minimum_required(VERSION 3.10)
project(SwarmApp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Find required packages
find_package(Threads REQUIRED)

# Include directories
include_directories(include)

# Core library
add_library(swarm-core
    src/core/message_bus.cpp
    src/core/module_manager.cpp
)

target_include_directories(swarm-core PUBLIC include)

# Main executable
add_executable(swarm-app 
    src/main.cpp
    src/modules/http-server/http_server_module.cpp
    src/modules/health-monitor/health_monitor_module.cpp
)

# Link libraries
target_link_libraries(swarm-app swarm-core Threads::Threads)

# Individual module executables
add_executable(http-server-module
    src/modules/http-server/http_server_standalone.cpp
    src/modules/http-server/http_server_module.cpp
    src/core/message_bus.cpp
    src/core/module_manager.cpp
)
target_link_libraries(http-server-module Threads::Threads)

add_executable(health-monitor-module
    src/modules/health-monitor/health_monitor_standalone.cpp
    src/modules/health-monitor/health_monitor_module.cpp
    src/core/message_bus.cpp
    src/core/module_manager.cpp
)
target_link_libraries(health-monitor-module Threads::Threads)

# Find Google Test (optional for production builds)
find_package(GTest QUIET)

# Tests (only if GTest is available)
if(GTest_FOUND)
    enable_testing()
    add_executable(test-swarm-app tests/test_main.cpp)
    target_link_libraries(test-swarm-app GTest::gtest GTest::gtest_main GTest::gmock Threads::Threads)
    
    # Add test
    add_test(NAME UnitTests COMMAND test-swarm-app)
endif()

# Installation
install(TARGETS swarm-app
        RUNTIME DESTINATION bin)

# Set version info
set_target_properties(swarm-app PROPERTIES
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME swarm-app
)
