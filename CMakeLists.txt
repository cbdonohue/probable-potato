cmake_minimum_required(VERSION 3.10)
project(SwarmApp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Find required packages
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

# Include directories
include_directories(include)
include_directories(${ZMQ_INCLUDE_DIRS})

# Core library
add_library(swarm-core
    src/core/message_bus.cpp
    src/core/module_manager.cpp
)

target_include_directories(swarm-core PUBLIC include)
target_link_libraries(swarm-core ${ZMQ_LIBRARIES})
target_compile_options(swarm-core PUBLIC ${ZMQ_CFLAGS_OTHER})

# Individual module libraries
add_library(swarm-http-server
    src/modules/http-server/http_server_module.cpp
)

target_link_libraries(swarm-http-server swarm-core Threads::Threads)
target_include_directories(swarm-http-server PUBLIC include)

add_library(swarm-health-monitor
    src/modules/health-monitor/health_monitor_module.cpp
)

target_link_libraries(swarm-health-monitor swarm-core Threads::Threads)
target_include_directories(swarm-health-monitor PUBLIC include)

# Main executable (links all modules)
add_executable(swarm-app 
    src/main.cpp
)

# Link all libraries to main executable
target_link_libraries(swarm-app 
    swarm-core 
    swarm-http-server 
    swarm-health-monitor 
    Threads::Threads
)

# Standalone module executables (for testing individual modules)
add_executable(http-server-standalone
    src/standalone/http_server_main.cpp
)

target_link_libraries(http-server-standalone 
    swarm-http-server 
    Threads::Threads
)

add_executable(health-monitor-standalone
    src/standalone/health_monitor_main.cpp
)

target_link_libraries(health-monitor-standalone 
    swarm-health-monitor 
    Threads::Threads
)

# Find Google Test (optional for production builds)
find_package(GTest QUIET)

# Tests (only if GTest is available)
if(GTest_FOUND)
    enable_testing()
    add_executable(test-swarm-app tests/test_main.cpp)
    target_link_libraries(test-swarm-app swarm-core GTest::gtest GTest::gtest_main GTest::gmock Threads::Threads)
    target_include_directories(test-swarm-app PUBLIC include)
    
    # Add ZeroMQ message bus test
add_executable(test-zeromq-message-bus tests/test_zeromq_message_bus.cpp)
target_link_libraries(test-zeromq-message-bus swarm-core GTest::gtest GTest::gtest_main GTest::gmock Threads::Threads)
target_include_directories(test-zeromq-message-bus PUBLIC include)
    
    # Add tests
    add_test(NAME UnitTests COMMAND test-swarm-app)
    add_test(NAME ZeroMQMessageBusTests COMMAND test-zeromq-message-bus)
endif()

# Installation
install(TARGETS swarm-app
        RUNTIME DESTINATION bin)

# Set version info
set_target_properties(swarm-app PROPERTIES
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME swarm-app
)
