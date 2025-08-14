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

add_library(swarm-health-monitor
    src/modules/health-monitor/health_monitor_module.cpp
)

target_link_libraries(swarm-health-monitor swarm-core Threads::Threads)
target_include_directories(swarm-health-monitor PUBLIC include)

# API Module (Oat++ based)
add_library(swarm-api
    src/modules/api/api_module.cpp
)

target_link_libraries(swarm-api swarm-core Threads::Threads oatpp::oatpp)
target_include_directories(swarm-api PUBLIC include)
target_include_directories(swarm-api PUBLIC /usr/local/include/oatpp-1.4.0)

# Main executable (links all modules)
add_executable(swarm-app 
    src/main.cpp
)

# Link all libraries to main executable
target_link_libraries(swarm-app 
    swarm-core 
    swarm-health-monitor 
    swarm-api
    Threads::Threads
)



add_executable(health-monitor-standalone
    src/standalone/health_monitor_main.cpp
)

target_link_libraries(health-monitor-standalone 
    swarm-health-monitor 
    Threads::Threads
    ${ZMQ_LIBRARIES}
)

add_executable(core-standalone
    src/standalone/core_main.cpp
)

target_link_libraries(core-standalone 
    swarm-core 
    Threads::Threads
    ${ZMQ_LIBRARIES}
)

add_executable(api-standalone
    src/standalone/api_main.cpp
)

target_link_libraries(api-standalone 
    swarm-api 
    Threads::Threads
    ${ZMQ_LIBRARIES}
)
target_include_directories(api-standalone PUBLIC /usr/local/include/oatpp-1.4.0)

# Find Google Test (optional for production builds)
find_package(GTest QUIET)

# Find CURL for HTTP testing
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)

# Find Oat++ framework
find_package(oatpp REQUIRED)

# Tests (only if GTest is available)
if(GTest_FOUND)
    enable_testing()
    
    # Main unit tests
    add_executable(test-swarm-app tests/test_main.cpp)
    target_link_libraries(test-swarm-app swarm-core GTest::gtest GTest::gtest_main GTest::gmock Threads::Threads ${ZMQ_LIBRARIES})
    target_include_directories(test-swarm-app PUBLIC include)
    
    # ZeroMQ message bus test
    add_executable(test-zeromq-message-bus tests/test_zeromq_message_bus.cpp)
    target_link_libraries(test-zeromq-message-bus swarm-core GTest::gtest GTest::gtest_main GTest::gmock Threads::Threads ${ZMQ_LIBRARIES})
    target_include_directories(test-zeromq-message-bus PUBLIC include)
    
    # Standalone applications test
    add_executable(test-standalone-apps tests/test_standalone_apps.cpp)
    target_link_libraries(test-standalone-apps 
        swarm-core 
        swarm-health-monitor 
        swarm-api
        GTest::gtest 
        GTest::gtest_main 
        GTest::gmock 
        Threads::Threads 
        ${ZMQ_LIBRARIES}
    )
    target_include_directories(test-standalone-apps PUBLIC include)
    target_include_directories(test-standalone-apps PUBLIC /usr/local/include/oatpp-1.4.0)
    target_link_libraries(test-standalone-apps ${CURL_LIBRARIES})
    target_include_directories(test-standalone-apps PUBLIC ${CURL_INCLUDE_DIRS})
    target_compile_options(test-standalone-apps PUBLIC ${CURL_CFLAGS_OTHER})
    
    # Individual standalone applications test
    add_executable(test-individual-standalone tests/test_individual_standalone.cpp)
    target_link_libraries(test-individual-standalone 
        swarm-core 
        swarm-health-monitor 
        swarm-api
        GTest::gtest 
        GTest::gtest_main 
        GTest::gmock 
        Threads::Threads 
        ${ZMQ_LIBRARIES}
    )
    target_include_directories(test-individual-standalone PUBLIC include)
    target_include_directories(test-individual-standalone PUBLIC /usr/local/include/oatpp-1.4.0)
    target_link_libraries(test-individual-standalone ${CURL_LIBRARIES})
    target_include_directories(test-individual-standalone PUBLIC ${CURL_INCLUDE_DIRS})
    target_compile_options(test-individual-standalone PUBLIC ${CURL_CFLAGS_OTHER})
    
    # Swarm integration test
    add_executable(test-swarm-integration tests/test_swarm_integration.cpp)
    target_link_libraries(test-swarm-integration 
        swarm-core 
        swarm-health-monitor 
        swarm-api
        GTest::gtest 
        GTest::gtest_main 
        GTest::gmock 
        Threads::Threads 
        ${ZMQ_LIBRARIES}
    )
    target_include_directories(test-swarm-integration PUBLIC include)
    target_include_directories(test-swarm-integration PUBLIC /usr/local/include/oatpp-1.4.0)
    target_link_libraries(test-swarm-integration ${CURL_LIBRARIES})
    target_include_directories(test-swarm-integration PUBLIC ${CURL_INCLUDE_DIRS})
    target_compile_options(test-swarm-integration PUBLIC ${CURL_CFLAGS_OTHER})
    
    # API module test
    add_executable(test-api-module tests/test_api_module.cpp)
    target_link_libraries(test-api-module 
        swarm-api 
        GTest::gtest 
        GTest::gtest_main 
        GTest::gmock 
        Threads::Threads 
        ${ZMQ_LIBRARIES}
    )
    target_include_directories(test-api-module PUBLIC include)
    target_include_directories(test-api-module PUBLIC /usr/local/include/oatpp-1.4.0)
    
    # Add all tests
    add_test(NAME UnitTests COMMAND test-swarm-app)
    add_test(NAME ZeroMQMessageBusTests COMMAND test-zeromq-message-bus)
    add_test(NAME StandaloneAppsTests COMMAND test-standalone-apps)
    add_test(NAME IndividualStandaloneTests COMMAND test-individual-standalone)
    add_test(NAME SwarmIntegrationTests COMMAND test-swarm-integration)
    add_test(NAME ApiModuleTests COMMAND test-api-module)
    
    # Test suite for all standalone and swarm tests
    add_test(NAME StandaloneAndSwarmTests 
        COMMAND ${CMAKE_CTEST_COMMAND} 
        --output-on-failure 
        -R "(StandaloneAppsTests|IndividualStandaloneTests|SwarmIntegrationTests)"
    )
    
else()
    message(WARNING "Google Test not found. Unit tests will not be built.")
endif()

# Installation
install(TARGETS swarm-app
        RUNTIME DESTINATION bin)

# Set version info
set_target_properties(swarm-app PROPERTIES
    VERSION ${PROJECT_VERSION}
    OUTPUT_NAME swarm-app
)

# Doxygen documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    # Configure the Doxyfile with correct paths
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    # Create output directory
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs/doxygen)
    
    # Add the target
    add_custom_target(docs ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
    
    # Install documentation
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/doxygen/html
            DESTINATION share/doc/swarmapp
            OPTIONAL)
else()
    message(WARNING "Doxygen not found. Documentation will not be generated.")
endif()
