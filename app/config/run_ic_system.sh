#!/bin/bash

# Run IC_Compositor on Jetson with IC Apps
# 
# Prerequisites:
#   1. Weston must be running (./run-jetson-wayland-full.sh)
#   2. Routing manager must be running

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}/../.."
APP_DIR="${SCRIPT_DIR}/.."

echo "========================================"
echo "IC Compositor + Apps Runner"
echo "========================================"

# Check if Weston is running
if ! sudo test -S "/tmp/xdg/wayland-0"; then
    echo "✗ Weston is not running!"
    echo "  Start it first: sudo ./run-jetson-wayland-full.sh"
    exit 1
fi
echo "✓ Weston is running"

# Check if routing manager is running
if ! pgrep -x "routingmanagerd" > /dev/null; then
    echo "✗ Routing manager is not running!"
    echo "  It should be started by run-jetson-wayland-full.sh"
    exit 1
fi
echo "✓ Routing manager is running"

# Check if IC_Compositor is built
if [ ! -f "${APP_DIR}/IC_Compositor/build/IC_Compositor" ]; then
    echo "✗ IC_Compositor not built!"
    echo "  Build it first: cd ${SCRIPT_DIR} && ./build_all_ic_apps.sh"
    exit 1
fi
echo "✓ IC_Compositor is built"

# Check if IC apps are built
IC_APPS=("BatteryMeter_app" "GearState_app" "Speedometer_app")
for APP in "${IC_APPS[@]}"; do
    if [ ! -f "${APP_DIR}/${APP}/build/${APP}" ]; then
        echo "✗ ${APP} not built!"
        echo "  Build it first: cd ${SCRIPT_DIR} && ./build_all_ic_apps.sh"
        exit 1
    fi
done
echo "✓ All IC apps are built"

# Kill existing IC processes
echo ""
echo "Cleaning up existing IC processes..."
sudo killall -9 IC_Compositor BatteryMeter_app GearState_app Speedometer_app 2>/dev/null || true
sleep 1

# Set up environment
export LD_LIBRARY_PATH="${PROJECT_ROOT}/install_folder/lib:${LD_LIBRARY_PATH}"
export XDG_RUNTIME_DIR="/tmp/xdg"
export QT_QPA_PLATFORM=wayland
export WAYLAND_DISPLAY=wayland-0

# Use software rendering for IC_Compositor
export QT_QUICK_BACKEND=software
export QSG_RENDER_LOOP=basic

echo ""
echo "Starting IC_Compositor..."
echo "  Display: wayland-0 (Weston)"
echo "  Creating: wayland-2 (for IC apps)"
echo "  Resolution: 1024x600"
echo ""

cd "${APP_DIR}/IC_Compositor/build"

# Start IC_Compositor in background
sudo -E XDG_RUNTIME_DIR=/tmp/xdg \
     QT_QPA_PLATFORM=wayland \
     WAYLAND_DISPLAY=wayland-0 \
     QT_QUICK_BACKEND=software \
     QSG_RENDER_LOOP=basic \
     LD_LIBRARY_PATH="${LD_LIBRARY_PATH}" \
     ./IC_Compositor > /tmp/ic_compositor.log 2>&1 &

IC_COMPOSITOR_PID=$!
echo "IC_Compositor PID: $IC_COMPOSITOR_PID"

# Wait for wayland-2 socket to be created
echo "Waiting for wayland-2 socket..."
for i in {1..10}; do
    if sudo test -S "/tmp/xdg/wayland-2"; then
        echo "✓ wayland-2 socket created"
        break
    fi
    sleep 1
    if [ $i -eq 10 ]; then
        echo "✗ wayland-2 socket not created!"
        echo "Check log: sudo cat /tmp/ic_compositor.log"
        sudo cat /tmp/ic_compositor.log
        exit 1
    fi
done

sleep 2

echo ""
echo "Starting IC apps in order..."
echo "  1. GearState_app (280x600, left)"
echo "  2. Speedometer_app (400x600, center)"
echo "  3. BatteryMeter_app (280x600, right)"
echo ""

# Function to start IC app
start_ic_app() {
    local APP_NAME=$1
    local APP_PATH="${APP_DIR}/${APP_NAME}"
    local CONFIG_DIR="${APP_PATH}/config"
    
    echo "Starting ${APP_NAME}..."
    
    cd "${APP_PATH}/build"
    
    # Start with software rendering and connect to wayland-2
    LOG_FILE="${APP_PATH}/ic_run.log"
    sudo -E XDG_RUNTIME_DIR=/tmp/xdg \
         QT_QPA_PLATFORM=wayland \
         WAYLAND_DISPLAY=wayland-2 \
         QT_QUICK_BACKEND=software \
         QSG_RENDER_LOOP=basic \
         LD_LIBRARY_PATH="${LD_LIBRARY_PATH}" \
         VSOMEIP_CONFIGURATION="${CONFIG_DIR}/vsomeip_${APP_NAME,,}.json" \
         COMMONAPI_CONFIG="${CONFIG_DIR}/commonapi_${APP_NAME,,}.ini" \
         ./${APP_NAME} > "${LOG_FILE}" 2>&1 &
    
    APP_PID=$!
    sleep 2
    
    if ! sudo ps -p $APP_PID > /dev/null 2>&1; then
        echo "✗ ${APP_NAME} failed to start"
        echo "  Log: ${LOG_FILE}"
        tail -20 "${LOG_FILE}"
        return 1
    fi
    
    echo "✓ ${APP_NAME} started (PID: $APP_PID)"
    return 0
}

# Start IC apps in order (important for index-based routing)
start_ic_app "GearState_app"
sleep 1
start_ic_app "Speedometer_app"
sleep 1
start_ic_app "BatteryMeter_app"

echo ""
echo "========================================"
echo "✅ IC System Status"
echo "========================================"
ps -p $IC_COMPOSITOR_PID > /dev/null && echo "✓ IC_Compositor" || echo "✗ IC_Compositor"
pgrep -x "GearState_app" > /dev/null && echo "✓ GearState_app" || echo "✗ GearState_app"
pgrep -x "Speedometer_app" > /dev/null && echo "✓ Speedometer_app" || echo "✗ Speedometer_app"
pgrep -x "BatteryMeter_app" > /dev/null && echo "✓ BatteryMeter_app" || echo "✗ BatteryMeter_app"

echo ""
echo "📋 Logs:"
echo "  Compositor: sudo cat /tmp/ic_compositor.log"
echo "  GearState:  cat ${APP_DIR}/GearState_app/ic_run.log"
echo "  Speedometer: cat ${APP_DIR}/Speedometer_app/ic_run.log"
echo "  BatteryMeter: cat ${APP_DIR}/BatteryMeter_app/ic_run.log"
echo ""
echo "🛑 Stop IC system:"
echo "  sudo killall -9 IC_Compositor BatteryMeter_app GearState_app Speedometer_app"
echo "========================================"
