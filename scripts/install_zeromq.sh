#!/bin/bash

# ZeroMQ Installation Script for SwarmApp
# This script installs ZeroMQ development libraries on various Linux distributions

set -e

echo "Installing ZeroMQ dependencies for SwarmApp..."

# Detect Linux distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$NAME
    VER=$VERSION_ID
else
    echo "Could not detect OS, trying to install anyway..."
    OS="Unknown"
fi

echo "Detected OS: $OS"

# Install ZeroMQ based on distribution
case $OS in
    *"Ubuntu"*|*"Debian"*)
        echo "Installing ZeroMQ on Ubuntu/Debian..."
        sudo apt-get update
        sudo apt-get install -y libzmq3-dev pkg-config
        ;;
    *"CentOS"*|*"Red Hat"*|*"Fedora"*)
        echo "Installing ZeroMQ on CentOS/RHEL/Fedora..."
        if command -v dnf &> /dev/null; then
            sudo dnf install -y zeromq-devel pkgconfig
        else
            sudo yum install -y zeromq-devel pkgconfig
        fi
        ;;
    *"Arch"*)
        echo "Installing ZeroMQ on Arch Linux..."
        sudo pacman -S --noconfirm zeromq pkg-config
        ;;
    *"Alpine"*)
        echo "Installing ZeroMQ on Alpine Linux..."
        sudo apk add --no-cache zeromq-dev pkgconfig
        ;;
    *)
        echo "Unknown distribution: $OS"
        echo "Please install ZeroMQ manually:"
        echo "  - Ubuntu/Debian: sudo apt-get install libzmq3-dev pkg-config"
        echo "  - CentOS/RHEL: sudo yum install zeromq-devel pkgconfig"
        echo "  - Arch: sudo pacman -S zeromq pkg-config"
        echo "  - Or build from source: https://github.com/zeromq/libzmq"
        exit 1
        ;;
esac

echo "ZeroMQ installation completed!"
echo ""
echo "To verify installation, run:"
echo "  pkg-config --modversion libzmq"
echo ""
echo "To build SwarmApp with ZeroMQ support:"
echo "  mkdir -p build && cd build"
echo "  cmake .."
echo "  make"
