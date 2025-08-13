#!/bin/bash

# Build and Run Individual Modules Script
# Usage: ./build-modules.sh [module-name] [action]

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Get the script directory and project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

# Available modules
MODULES=("http-server" "health-monitor" "all")

# Build a specific module
build_module() {
    local module=$1
    log_info "Building $module module..."
    
    cd "$PROJECT_ROOT"
    
    # Create build directory if it doesn't exist
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    
    # Run cmake if needed
    if [ ! -f "Makefile" ]; then
        log_info "Running cmake..."
        cmake ..
    fi
    
    # Build the module
    if [ "$module" = "all" ]; then
        make -j$(nproc)
    else
        make "${module}-module" -j$(nproc)
    fi
    
    log_success "$module module built successfully"
}

# Run a specific module
run_module() {
    local module=$1
    local executable="$BUILD_DIR/${module}-module"
    
    if [ ! -f "$executable" ]; then
        log_error "Module $module not built. Run 'build' first."
        exit 1
    fi
    
    log_info "Running $module module..."
    log_info "Press Ctrl+C to stop"
    
    cd "$BUILD_DIR"
    ./"${module}-module"
}

# Show available modules
show_modules() {
    echo "Available modules:"
    for module in "${MODULES[@]}"; do
        echo "  - $module"
    done
    echo ""
    echo "Available actions:"
    echo "  - build [module]  - Build a specific module"
    echo "  - run [module]    - Run a specific module"
    echo "  - clean           - Clean build directory"
    echo "  - list            - List available modules"
}

# Clean build directory
clean_build() {
    log_info "Cleaning build directory..."
    cd "$PROJECT_ROOT"
    rm -rf "$BUILD_DIR"
    log_success "Build directory cleaned"
}

# Main execution
case "${1:-}" in
    "build")
        if [ -z "${2:-}" ]; then
            log_error "Please specify a module to build"
            show_modules
            exit 1
        fi
        
        # Check if module is valid
        if [[ ! " ${MODULES[@]} " =~ " ${2} " ]]; then
            log_error "Invalid module: $2"
            show_modules
            exit 1
        fi
        
        build_module "$2"
        ;;
        
    "run")
        if [ -z "${2:-}" ]; then
            log_error "Please specify a module to run"
            show_modules
            exit 1
        fi
        
        # Check if module is valid (exclude "all")
        if [ "$2" = "all" ]; then
            log_error "Cannot run 'all' module. Please specify a specific module."
            exit 1
        fi
        
        if [[ ! " ${MODULES[@]} " =~ " ${2} " ]]; then
            log_error "Invalid module: $2"
            show_modules
            exit 1
        fi
        
        run_module "$2"
        ;;
        
    "clean")
        clean_build
        ;;
        
    "list")
        show_modules
        ;;
        
    "help"|"-h"|"--help")
        echo "Usage: $0 [action] [module]"
        echo ""
        show_modules
        echo "Examples:"
        echo "  $0 build http-server"
        echo "  $0 run http-server"
        echo "  $0 build all"
        echo "  $0 clean"
        ;;
        
    *)
        log_error "Unknown action: ${1:-}"
        echo ""
        show_modules
        exit 1
        ;;
esac
