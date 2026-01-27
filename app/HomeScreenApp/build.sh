#!/bin/bash

set -e

echo "========================================="
echo "  HomeScreenApp - BUILD"
echo "  (vsomeip Client @ ECU2 192.168.1.101)"
echo "========================================="
echo ""

# CommonAPI generated code directory
if [ -z "$COMMONAPI_GEN_DIR" ]; then
    export COMMONAPI_GEN_DIR=$(pwd)/../../commonapi/generated
fi

echo "📂 CommonAPI generated code: $COMMONAPI_GEN_DIR"
echo ""

# Clean previous build
if [ -d "build" ]; then
    echo "🧹 Cleaning previous build..."
    rm -rf build
fi

mkdir -p build
cd build

echo "🔧 Running CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_COMPILER=gcc \
    -DCOMMONAPI_GEN_DIR=$COMMONAPI_GEN_DIR

echo ""
echo "🔨 Building..."
make -j$(nproc)

echo ""
echo "========================================="
echo "✅ Build Successful!"
echo "========================================="
echo "Executable: build/HomeScreenApp"
echo ""
echo "To run: ./run.sh"
echo "========================================="
