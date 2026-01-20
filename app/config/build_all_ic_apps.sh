#!/bin/bash

# Build All IC Apps for Jetson Orin Nano
# Builds BatteryMeter, GearState, and Speedometer apps

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_DIR="${SCRIPT_DIR}/.."

echo "========================================"
echo "Building All IC Apps"
echo "========================================"

# Array of IC apps
IC_APPS=(
    "BatteryMeter_app"
    "GearState_app"
    "Speedometer_app"
)

# Track success
BUILT_APPS=()
FAILED_APPS=()

# Build each app
for APP in "${IC_APPS[@]}"; do
    echo ""
    echo "----------------------------------------"
    echo "Building ${APP}..."
    echo "----------------------------------------"
    
    APP_PATH="${APP_DIR}/${APP}"
    
    if [ ! -d "${APP_PATH}" ]; then
        echo "ERROR: ${APP} directory not found at ${APP_PATH}"
        FAILED_APPS+=("${APP}")
        continue
    fi
    
    cd "${APP_PATH}"
    
    if [ ! -f "build.sh" ]; then
        echo "ERROR: build.sh not found in ${APP_PATH}"
        FAILED_APPS+=("${APP}")
        continue
    fi
    
    # Make build script executable
    chmod +x build.sh
    
    # Build the app
    if ./build.sh "$@"; then
        BUILT_APPS+=("${APP}")
        echo "✓ ${APP} built successfully"
    else
        FAILED_APPS+=("${APP}")
        echo "✗ ${APP} build failed"
    fi
done

# Summary
echo ""
echo "========================================"
echo "Build Summary"
echo "========================================"
echo "Successfully built: ${#BUILT_APPS[@]}"
for APP in "${BUILT_APPS[@]}"; do
    echo "  ✓ ${APP}"
done

if [ ${#FAILED_APPS[@]} -gt 0 ]; then
    echo ""
    echo "Failed to build: ${#FAILED_APPS[@]}"
    for APP in "${FAILED_APPS[@]}"; do
        echo "  ✗ ${APP}"
    done
    exit 1
fi

echo ""
echo "All IC apps built successfully!"
