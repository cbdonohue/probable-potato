#!/bin/bash

# SwarmApp Test Runner Script
# This script builds and runs all tests for the SwarmApp project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    # Check for required commands
    local missing_deps=()
    
    if ! command -v cmake &> /dev/null; then
        missing_deps+=("cmake")
    fi
    
    if ! command -v make &> /dev/null; then
        missing_deps+=("make")
    fi
    
    if ! command -v pkg-config &> /dev/null; then
        missing_deps+=("pkg-config")
    fi
    
    # Check for required libraries
    if ! pkg-config --exists libzmq; then
        missing_deps+=("libzmq3-dev")
    fi
    
    if ! pkg-config --exists libcurl; then
        missing_deps+=("libcurl4-openssl-dev")
    fi
    
    if [ ${#missing_deps[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing_deps[*]}"
        print_status "Run './scripts/install_dependencies.sh' to install them"
        exit 1
    fi
    
    print_success "All dependencies found"
}

# Function to build the project
build_project() {
    print_status "Building project..."
    
    # Create build directory if it doesn't exist
    mkdir -p build
    
    # Configure with CMake
    cd build
    cmake ..
    
    # Build
    make -j$(nproc)
    
    cd ..
    print_success "Build completed"
}

# Function to run all tests
run_tests() {
    print_status "Running all tests..."
    
    cd build
    
    # Run tests with verbose output
    if ctest --output-on-failure --verbose; then
        print_success "All tests passed!"
    else
        print_error "Some tests failed"
        exit 1
    fi
    
    cd ..
}

# Function to run specific test suites
run_specific_tests() {
    local test_suite=$1
    
    print_status "Running $test_suite tests..."
    
    cd build
    
    case $test_suite in
        "standalone")
            ./test-standalone-apps --gtest_brief=1
            ./test-individual-standalone --gtest_brief=1
            ;;
        "swarm")
            ./test-swarm-integration --gtest_brief=1
            ;;
        "core")
            ./test-swarm-app --gtest_brief=1
            ./test-zeromq-message-bus --gtest_brief=1
            ;;
        *)
            print_error "Unknown test suite: $test_suite"
            print_status "Available test suites: standalone, swarm, core"
            exit 1
            ;;
    esac
    
    cd ..
    print_success "$test_suite tests completed"
}

# Function to clean build
clean_build() {
    print_status "Cleaning build directory..."
    rm -rf build
    print_success "Build directory cleaned"
}

# Function to show help
show_help() {
    echo "SwarmApp Test Runner"
    echo ""
    echo "Usage: $0 [OPTIONS] [TEST_SUITE]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -c, --clean         Clean build directory before building"
    echo "  -d, --deps          Check dependencies only"
    echo "  -b, --build         Build project only"
    echo ""
    echo "Test Suites:"
    echo "  standalone          Run standalone application tests"
    echo "  swarm              Run swarm integration tests"
    echo "  core               Run core functionality tests"
    echo ""
    echo "Examples:"
    echo "  $0                  # Run all tests"
    echo "  $0 standalone       # Run standalone tests only"
    echo "  $0 -c              # Clean build and run all tests"
    echo "  $0 -b              # Build only"
}

# Main script logic
main() {
    local clean_build_flag=false
    local check_deps_only=false
    local build_only=false
    local test_suite=""
    
    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -c|--clean)
                clean_build_flag=true
                shift
                ;;
            -d|--deps)
                check_deps_only=true
                shift
                ;;
            -b|--build)
                build_only=true
                shift
                ;;
            -*)
                print_error "Unknown option: $1"
                show_help
                exit 1
                ;;
            *)
                if [ -z "$test_suite" ]; then
                    test_suite=$1
                else
                    print_error "Multiple test suites specified"
                    exit 1
                fi
                shift
                ;;
        esac
    done
    
    # Check dependencies
    check_dependencies
    
    if [ "$check_deps_only" = true ]; then
        exit 0
    fi
    
    # Clean build if requested
    if [ "$clean_build_flag" = true ]; then
        clean_build
    fi
    
    # Build project
    build_project
    
    if [ "$build_only" = true ]; then
        exit 0
    fi
    
    # Run tests
    if [ -n "$test_suite" ]; then
        run_specific_tests "$test_suite"
    else
        run_tests
    fi
}

# Run main function with all arguments
main "$@"
