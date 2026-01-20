#!/bin/bash

# Test IC Apps on Jetson Orin Nano
# Tests BatteryMeter, GearState, and Speedometer apps with vsomeip communication
# 
# Prerequisites:
#   1. Run ./run-jetson-wayland-full.sh first (to start Weston)
#   2. Routing manager must be running (run ./start_routing_manager.sh if not)

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_DIR="${SCRIPT_DIR}/.."
PROJECT_ROOT="${SCRIPT_DIR}/../.."

echo "========================================"
echo "IC Apps Test Runner"
echo "========================================"

# Check if apps are built
IC_APPS=(
    "BatteryMeter_app"
    "GearState_app"
    "Speedometer_app"
)

echo "Checking if apps are built..."
ALL_BUILT=true
for APP in "${IC_APPS[@]}"; do
    if [ ! -f "${APP_DIR}/${APP}/build/${APP}" ]; then
        echo "✗ ${APP} not built"
        ALL_BUILT=false
    else
        echo "✓ ${APP} found"
    fi
done

if [ "$ALL_BUILT" = false ]; then
    echo ""
    echo "Some apps are not built. Run build_all_ic_apps.sh first."
    exit 1
fi

# Check for routing manager (should be running already)
if ! pgrep -x "routingmanagerd" > /dev/null; then
    echo "✗ Routing manager is not running!"
    echo "  Start it first: ./start_routing_manager.sh"
    exit 1
fi
echo "✓ Routing manager is running"

# Kill any existing IC app processes
echo ""
echo "Cleaning up existing IC app processes..."
sudo killall -9 BatteryMeter_app GearState_app Speedometer_app 2>/dev/null || true
sleep 1

# Set up environment
export LD_LIBRARY_PATH="${PROJECT_ROOT}/install_folder/lib:${LD_LIBRARY_PATH}"

# Set up Wayland environment (connect to Weston wayland-0, not HU compositor wayland-1)
export XDG_RUNTIME_DIR="/tmp/xdg"
export QT_QPA_PLATFORM=wayland
export WAYLAND_DISPLAY=wayland-0

# Check if Weston is running (need sudo to check)
if ! sudo test -S "${XDG_RUNTIME_DIR}/wayland-0"; then
    echo "✗ Weston is not running!"
    echo "  Start it first: sudo ./run-jetson-wayland-full.sh"
    exit 1
fi
echo "✓ Weston is running (${XDG_RUNTIME_DIR}/wayland-0)"

# Function to start an app
start_app() {
    local APP_NAME=$1
    local APP_PATH="${APP_DIR}/${APP_NAME}"
    local CONFIG_DIR="${APP_PATH}/config"
    
    echo ""
    echo "Starting ${APP_NAME}..."
    
    # Set app-specific config
    export VSOMEIP_CONFIGURATION="${CONFIG_DIR}/vsomeip_${APP_NAME,,}.json"
    export COMMONAPI_CONFIG="${CONFIG_DIR}/commonapi_${APP_NAME,,}.ini"
    
    cd "${APP_PATH}/build"
    
    # Start the app with sudo (required for Wayland access)
    # Use wayland-0 (Weston) not wayland-1 (HU compositor)
    # Use software rendering to avoid GPU sync overhead
    LOG_FILE="${APP_PATH}/test_run.log"
    sudo XDG_RUNTIME_DIR=/tmp/xdg \
         QT_QPA_PLATFORM=wayland \
         WAYLAND_DISPLAY=wayland-0 \
         QT_QUICK_BACKEND=software \
         QSG_RENDER_LOOP=basic \
         LD_LIBRARY_PATH="${LD_LIBRARY_PATH}" \
         VSOMEIP_CONFIGURATION="${VSOMEIP_CONFIGURATION}" \
         COMMONAPI_CONFIG="${COMMONAPI_CONFIG}" \
         ./${APP_NAME} > "${LOG_FILE}" 2>&1 &
    
    APP_PID=$!
    
    sleep 2
    
    if ! sudo ps -p $APP_PID > /dev/null 2>&1; then
        echo "✗ ${APP_NAME} failed to start"
        echo "  Check log: ${LOG_FILE}"
        tail -20 "${LOG_FILE}"
        return 1
    fi
    
    echo "✓ ${APP_NAME} started (PID: $APP_PID, Log: ${LOG_FILE})"
    return 0
}

# Start all IC apps
echo ""
echo "Starting IC apps..."
STARTED_APPS=()
FAILED_APPS=()

for APP in "${IC_APPS[@]}"; do
    if start_app "${APP}"; then
        STARTED_APPS+=("${APP}")
    else
        FAILED_APPS+=("${APP}")
    fi
done

# Summary
echo ""
echo "========================================"
echo "Test Status"
echo "========================================"
echo "Routing Manager: Running"
echo ""
echo "Started apps: ${#STARTED_APPS[@]}"
for APP in "${STARTED_APPS[@]}"; do
    echo "  ✓ ${APP}"
done

if [ ${#FAILED_APPS[@]} -gt 0 ]; then
    echo ""
    echo "Failed to start: ${#FAILED_APPS[@]}"
    for APP in "${FAILED_APPS[@]}"; do
        echo "  ✗ ${APP}"
    done
fi

echo ""
echo "========================================"
echo "Monitor Instructions"
echo "========================================"
echo "1. Check app windows on display - all 3 IC apps should be visible"
echo "2. IC apps are connected to Weston (wayland-0) directly"
echo "3. HU apps are connected to HU_MainApp_Compositor (wayland-1)"
echo ""
echo "4. Check vsomeip communication in logs:"
echo "   tail -f ${APP_DIR}/BatteryMeter_app/test_run.log"
echo "   tail -f ${APP_DIR}/GearState_app/test_run.log"
echo "   tail -f ${APP_DIR}/Speedometer_app/test_run.log"
echo ""
echo "5. To stop IC apps only:"
echo "   sudo killall -9 BatteryMeter_app GearState_app Speedometer_app"
echo ""
echo "6. To stop routing manager (will affect HU apps too):"
echo "   killall -9 routingmanagerd"
echo ""
echo "========================================"
