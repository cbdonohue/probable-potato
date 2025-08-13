#!/bin/bash

# SwarmApp Dependencies Installation Script
# This script installs all required dependencies for building and testing SwarmApp

set -e  # Exit on any error

echo "Installing SwarmApp dependencies..."

# Detect the operating system
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    if command -v apt-get &> /dev/null; then
        # Debian/Ubuntu
        echo "Detected Debian/Ubuntu system"
        sudo apt-get update
        sudo apt-get install -y \
            build-essential \
            cmake \
            pkg-config \
            libzmq3-dev \
            libcurl4-openssl-dev \
            libgtest-dev \
            libgmock-dev
    elif command -v yum &> /dev/null; then
        # CentOS/RHEL/Fedora
        echo "Detected CentOS/RHEL/Fedora system"
        sudo yum update -y
        sudo yum install -y \
            gcc-c++ \
            cmake \
            pkg-config \
            zeromq-devel \
            libcurl-devel \
            gtest-devel \
            gmock-devel
    elif command -v dnf &> /dev/null; then
        # Fedora (newer versions)
        echo "Detected Fedora system"
        sudo dnf update -y
        sudo dnf install -y \
            gcc-c++ \
            cmake \
            pkg-config \
            zeromq-devel \
            libcurl-devel \
            gtest-devel \
            gmock-devel
    else
        echo "Unsupported Linux distribution. Please install dependencies manually:"
        echo "- build-essential (or gcc-c++)"
        echo "- cmake"
        echo "- pkg-config"
        echo "- libzmq3-dev (or zeromq-devel)"
        echo "- libcurl4-openssl-dev (or libcurl-devel)"
        echo "- libgtest-dev (or gtest-devel)"
        echo "- libgmock-dev (or gmock-devel)"
        exit 1
    fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    echo "Detected macOS system"
    if command -v brew &> /dev/null; then
        brew update
        brew install \
            cmake \
            pkg-config \
            zeromq \
            curl \
            googletest
    else
        echo "Homebrew not found. Please install Homebrew first:"
        echo "https://brew.sh/"
        exit 1
    fi
else
    echo "Unsupported operating system: $OSTYPE"
    echo "Please install dependencies manually:"
    echo "- C++ compiler (GCC or Clang)"
    echo "- CMake"
    echo "- pkg-config"
    echo "- ZeroMQ development libraries"
    echo "- CURL development libraries"
    echo "- Google Test"
    exit 1
fi

echo "Dependencies installed successfully!"
echo ""
echo "You can now build the project with:"
echo "  mkdir -p build && cd build"
echo "  cmake .."
echo "  make"
echo ""
echo "And run tests with:"
echo "  make test"
