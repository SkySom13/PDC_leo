#!/bin/bash

# ════════════════════════════════════════════════════════════
# Build All HU Apps - Jetson Orin Nano
# ════════════════════════════════════════════════════════════

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
DEPLOY_PREFIX="${PROJECT_ROOT}/install_folder"

echo "════════════════════════════════════════════════════════════"
echo "Building All HU Apps"
echo "════════════════════════════════════════════════════════════"
echo "Project Root: ${PROJECT_ROOT}"
echo "Deploy Prefix: ${DEPLOY_PREFIX}"
echo ""

export CMAKE_PREFIX_PATH="${DEPLOY_PREFIX}"
export LD_LIBRARY_PATH="${DEPLOY_PREFIX}/lib:${LD_LIBRARY_PATH}"

BUILD_FAILED=0

# 1. HU_MainApp_Compositor
echo "[1/5] Building HU_MainApp_Compositor..."
cd "${PROJECT_ROOT}/app/HU_MainApp"
./build_compositor.sh
if [ $? -ne 0 ]; then
    echo "❌ HU_MainApp_Compositor build failed!"
    BUILD_FAILED=1
else
    echo "✓ HU_MainApp_Compositor built"
fi
echo ""

# 2. GearApp
echo "[2/5] Building GearApp..."
cd "${PROJECT_ROOT}/app/GearApp"
./build.sh
if [ $? -ne 0 ]; then
    echo "❌ GearApp build failed!"
    BUILD_FAILED=1
else
    echo "✓ GearApp built"
fi
echo ""

# 3. MediaApp
echo "[3/5] Building MediaApp..."
cd "${PROJECT_ROOT}/app/MediaApp"
./build.sh
if [ $? -ne 0 ]; then
    echo "❌ MediaApp build failed!"
    BUILD_FAILED=1
else
    echo "✓ MediaApp built"
fi
echo ""

# 4. AmbientApp
echo "[4/5] Building AmbientApp..."
cd "${PROJECT_ROOT}/app/AmbientApp"
./build.sh
if [ $? -ne 0 ]; then
    echo "❌ AmbientApp build failed!"
    BUILD_FAILED=1
else
    echo "✓ AmbientApp built"
fi
echo ""

# 5. HomeScreenApp
echo "[5/5] Building HomeScreenApp..."
cd "${PROJECT_ROOT}/app/HomeScreenApp"
./build.sh
if [ $? -ne 0 ]; then
    echo "❌ HomeScreenApp build failed!"
    BUILD_FAILED=1
else
    echo "✓ HomeScreenApp built"
fi
echo ""

echo "════════════════════════════════════════════════════════════"
if [ $BUILD_FAILED -eq 0 ]; then
    echo "✅ All HU apps built successfully!"
else
    echo "❌ Some apps failed to build. Check logs above."
fi
echo "════════════════════════════════════════════════════════════"

exit $BUILD_FAILED
