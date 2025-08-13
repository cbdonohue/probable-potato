#!/bin/bash

# SwarmApp Documentation Generator
# This script generates Doxygen documentation for the SwarmApp project

set -e

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

# Get the script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

print_status "SwarmApp Documentation Generator"
print_status "Project root: $PROJECT_ROOT"

# Check if we're in the right directory
if [[ ! -f "$PROJECT_ROOT/CMakeLists.txt" ]]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root or scripts directory."
    exit 1
fi

# Check if Doxygen is installed
if ! command -v doxygen &> /dev/null; then
    print_error "Doxygen is not installed. Please install it first:"
    echo "  Ubuntu/Debian: sudo apt install doxygen"
    echo "  CentOS/RHEL: sudo yum install doxygen"
    echo "  macOS: brew install doxygen"
    exit 1
fi

print_status "Doxygen version: $(doxygen --version)"

# Parse command line arguments
BUILD_TYPE="cmake"
CLEAN_BUILD=false
OPEN_BROWSER=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --direct)
            BUILD_TYPE="direct"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --open)
            OPEN_BROWSER=true
            shift
            ;;
        --help|-h)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --direct     Use Doxygen directly instead of CMake"
            echo "  --clean      Clean build directory before generating"
            echo "  --open       Open documentation in browser after generation"
            echo "  --help, -h   Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                    # Generate docs using CMake"
            echo "  $0 --direct           # Generate docs using Doxygen directly"
            echo "  $0 --clean --open     # Clean build and open in browser"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Change to project root
cd "$PROJECT_ROOT"

# Clean build if requested
if [[ "$CLEAN_BUILD" == true ]]; then
    print_status "Cleaning build directory..."
    rm -rf build/
    print_success "Build directory cleaned"
fi

# Generate documentation
if [[ "$BUILD_TYPE" == "cmake" ]]; then
    print_status "Generating documentation using CMake..."
    
    # Create build directory if it doesn't exist
    mkdir -p build
    cd build
    
    # Configure and build
    cmake .. > /dev/null
    make docs
    
    print_success "Documentation generated successfully using CMake"
    DOCS_PATH="$PROJECT_ROOT/docs/doxygen/html/index.html"
    
else
    print_status "Generating documentation using Doxygen directly..."
    
    # Generate documentation directly
    doxygen Doxyfile
    
    print_success "Documentation generated successfully using Doxygen"
    DOCS_PATH="$PROJECT_ROOT/docs/doxygen/html/index.html"
fi

# Check if documentation was generated
if [[ -f "$DOCS_PATH" ]]; then
    print_success "Documentation is available at: $DOCS_PATH"
    
    # Open in browser if requested
    if [[ "$OPEN_BROWSER" == true ]]; then
        print_status "Opening documentation in browser..."
        if command -v xdg-open &> /dev/null; then
            xdg-open "$DOCS_PATH"
        elif command -v open &> /dev/null; then
            open "$DOCS_PATH"
        else
            print_warning "Could not automatically open browser. Please open manually:"
            echo "  $DOCS_PATH"
        fi
    fi
else
    print_error "Documentation was not generated successfully"
    exit 1
fi

print_success "Documentation generation completed!"
