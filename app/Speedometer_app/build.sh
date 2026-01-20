#!/bin/bash

# Speedometer_app Build Script for Jetson
# Builds the IC Speedometer module

set -e  # Exit on error

APP_NAME="Speedometer_app"
BUILD_DIR="build"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "========================================"
echo "Building ${APP_NAME}"
echo "========================================"

cd "${SCRIPT_DIR}"

# Clean build directory if requested
if [ "$1" == "clean" ]; then
    echo "Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
fi

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure with CMake
echo "Configuring with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake

# Build
echo "Building..."
make -j$(nproc)

echo "========================================"
echo "${APP_NAME} build complete!"
echo "Executable: ${SCRIPT_DIR}/${BUILD_DIR}/${APP_NAME}"
echo "========================================"
