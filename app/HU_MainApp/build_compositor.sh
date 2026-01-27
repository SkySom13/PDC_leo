#!/bin/bash

echo "════════════════════════════════════════════════════════"
echo "Building HU_MainApp - Wayland Compositor Only"
echo "════════════════════════════════════════════════════════"

cd "$(dirname "$0")"

# Clean build
echo "Cleaning build directory..."
rm -rf build_compositor
mkdir -p build_compositor
cd build_compositor

# CMake with compositor-only CMakeLists
echo "Running CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Debug
if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed!"
    exit 1
fi

# Build
echo "Building..."
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

echo "✅ Build successful!"
echo ""
echo "To run HU_MainApp Compositor:"
echo "  ./run_compositor.sh"
echo ""
echo "Note: This is a Wayland compositor only."
echo "Run independent apps separately:"
echo "  - cd app/GearApp && ./run.sh"
echo "  - cd app/MediaApp && ./run.sh"
echo "  - cd app/AmbientApp && ./run.sh"
echo "════════════════════════════════════════════════════════"
