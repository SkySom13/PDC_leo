#!/bin/bash

# ═══════════════════════════════════════════════════════
# IC System Debug Script
# Captures all output to log files for analysis
# ═══════════════════════════════════════════════════════

# Setup environment
export LD_LIBRARY_PATH="${HOME}/Qt/5.15.2/gcc_64/lib:${LD_LIBRARY_PATH}"
export XDG_RUNTIME_DIR="/tmp/runtime-${USER}"
mkdir -p ${XDG_RUNTIME_DIR}
chmod 0700 ${XDG_RUNTIME_DIR}

# Log directory
LOG_DIR="/tmp/ic_debug_logs"
rm -rf ${LOG_DIR}
mkdir -p ${LOG_DIR}

# Clean up existing processes and sockets
rm -f ${XDG_RUNTIME_DIR}/wayland-*
pkill -9 -f IC_Compositor 2>/dev/null
pkill -9 -f GearState_app 2>/dev/null
pkill -9 -f Speedometer_app 2>/dev/null
pkill -9 -f BatteryMeter_app 2>/dev/null
sleep 1

# Application paths
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMPOSITOR_PATH="${BASE_DIR}/IC_Compositor/build/IC_Compositor"
GEARSTATE_PATH="${BASE_DIR}/GearState_app/build/GearState_app"
SPEEDOMETER_PATH="${BASE_DIR}/Speedometer_app/build/Speedometer_app"
BATTERYMETER_PATH="${BASE_DIR}/BatteryMeter_app/build/BatteryMeter_app"

# ═══════════════════════════════════════════════════════
# Print Header
# ═══════════════════════════════════════════════════════
clear
echo "═══════════════════════════════════════════════════════"
echo "IC System Debug Mode"
echo "═══════════════════════════════════════════════════════"
echo ""
echo "Log Directory: ${LOG_DIR}"
echo "Runtime Dir:   ${XDG_RUNTIME_DIR}"
echo ""
echo "═══════════════════════════════════════════════════════"
echo ""

# ═══════════════════════════════════════════════════════
# Check if applications exist
# ═══════════════════════════════════════════════════════
echo "Checking applications..."
echo ""

if [ ! -f "${COMPOSITOR_PATH}" ]; then
    echo "✗ Compositor not found: ${COMPOSITOR_PATH}"
    exit 1
else
    echo "✓ Compositor found"
fi

if [ ! -f "${GEARSTATE_PATH}" ]; then
    echo "✗ GearState not found: ${GEARSTATE_PATH}"
else
    echo "✓ GearState found"
fi

if [ ! -f "${SPEEDOMETER_PATH}" ]; then
    echo "✗ Speedometer not found: ${SPEEDOMETER_PATH}"
else
    echo "✓ Speedometer found"
fi

if [ ! -f "${BATTERYMETER_PATH}" ]; then
    echo "✗ BatteryMeter not found: ${BATTERYMETER_PATH}"
else
    echo "✓ BatteryMeter found"
fi

echo ""
echo "═══════════════════════════════════════════════════════"
echo ""

# ═══════════════════════════════════════════════════════
# Start Compositor with full debug logging
# ═══════════════════════════════════════════════════════
echo "Starting IC Compositor with debug logging..."

QT_QPA_PLATFORM=xcb \
QT_LOGGING_RULES="*.debug=true;qt.qpa.*=true;qt.waylandcompositor.*=true;qt.qml.*=true" \
QT_DEBUG_PLUGINS=1 \
${COMPOSITOR_PATH} > ${LOG_DIR}/compositor_stdout.log 2> ${LOG_DIR}/compositor_stderr.log &

COMPOSITOR_PID=$!
echo "  PID: ${COMPOSITOR_PID}"
echo "  Logs: ${LOG_DIR}/compositor_*.log"
sleep 3

# Check if compositor is still running
if ! ps -p ${COMPOSITOR_PID} > /dev/null; then
    echo ""
    echo "✗ ERROR: Compositor crashed or failed to start!"
    echo ""
    echo "════════════════════ STDOUT ════════════════════"
    cat ${LOG_DIR}/compositor_stdout.log
    echo ""
    echo "════════════════════ STDERR ════════════════════"
    cat ${LOG_DIR}/compositor_stderr.log
    echo "═══════════════════════════════════════════════════════"
    exit 1
fi

echo "  ✓ Compositor running"
echo ""

# ═══════════════════════════════════════════════════════
# Wait for Wayland socket
# ═══════════════════════════════════════════════════════
WAYLAND_DISPLAY="wayland-0"
SOCKET_PATH="${XDG_RUNTIME_DIR}/${WAYLAND_DISPLAY}"

echo "Waiting for Wayland socket..."
timeout=15
while [ ! -S "${SOCKET_PATH}" ] && [ $timeout -gt 0 ]; do
    sleep 1
    echo -n "."
    timeout=$((timeout-1))
done
echo ""

if [ ! -S "${SOCKET_PATH}" ]; then
    echo ""
    echo "✗ ERROR: Wayland socket not created after 15 seconds!"
    echo ""
    echo "════════════════════ COMPOSITOR STDOUT ════════════════════"
    cat ${LOG_DIR}/compositor_stdout.log
    echo ""
    echo "════════════════════ COMPOSITOR STDERR ════════════════════"
    cat ${LOG_DIR}/compositor_stderr.log
    echo "═══════════════════════════════════════════════════════"
    kill ${COMPOSITOR_PID} 2>/dev/null
    exit 1
fi

echo "  ✓ Socket ready: ${SOCKET_PATH}"
ls -la ${SOCKET_PATH} > ${LOG_DIR}/socket_info.log
echo ""
echo "═══════════════════════════════════════════════════════"
echo ""

# ═══════════════════════════════════════════════════════
# Start Client Applications
# ═══════════════════════════════════════════════════════
echo "Starting client applications with debug logging..."
echo ""

# GearState
if [ -f "${GEARSTATE_PATH}" ]; then
    echo "  → GearState (App ID: appGearState, IVI ID: 20001)"
    
    QT_QPA_PLATFORM=wayland \
    WAYLAND_DISPLAY=${WAYLAND_DISPLAY} \
    QT_LOGGING_RULES="*.debug=true;qt.qpa.wayland*=true;qt.qml.*=true" \
    QT_DEBUG_PLUGINS=1 \
    IVISURFACEID=20001 \
    ${GEARSTATE_PATH} > ${LOG_DIR}/gearstate_stdout.log 2> ${LOG_DIR}/gearstate_stderr.log &
    
    GEARSTATE_PID=$!
    echo "     PID: ${GEARSTATE_PID}"
    echo "     Logs: ${LOG_DIR}/gearstate_*.log"
    sleep 2
fi

# Speedometer
if [ -f "${SPEEDOMETER_PATH}" ]; then
    echo "  → Speedometer (App ID: appSpeedometer, IVI ID: 20002)"
    
    QT_QPA_PLATFORM=wayland \
    WAYLAND_DISPLAY=${WAYLAND_DISPLAY} \
    QT_LOGGING_RULES="*.debug=true;qt.qpa.wayland*=true;qt.qml.*=true" \
    QT_DEBUG_PLUGINS=1 \
    IVISURFACEID=20002 \
    ${SPEEDOMETER_PATH} > ${LOG_DIR}/speedometer_stdout.log 2> ${LOG_DIR}/speedometer_stderr.log &
    
    SPEEDOMETER_PID=$!
    echo "     PID: ${SPEEDOMETER_PID}"
    echo "     Logs: ${LOG_DIR}/speedometer_*.log"
    sleep 2
fi

# BatteryMeter
if [ -f "${BATTERYMETER_PATH}" ]; then
    echo "  → BatteryMeter (App ID: appBatteryMeter, IVI ID: 20003)"
    
    QT_QPA_PLATFORM=wayland \
    WAYLAND_DISPLAY=${WAYLAND_DISPLAY} \
    QT_LOGGING_RULES="*.debug=true;qt.qpa.wayland*=true;qt.qml.*=true" \
    QT_DEBUG_PLUGINS=1 \
    IVISURFACEID=20003 \
    ${BATTERYMETER_PATH} > ${LOG_DIR}/batterymeter_stdout.log 2> ${LOG_DIR}/batterymeter_stderr.log &
    
    BATTERYMETER_PID=$!
    echo "     PID: ${BATTERYMETER_PID}"
    echo "     Logs: ${LOG_DIR}/batterymeter_*.log"
    sleep 2
fi

echo ""
echo "═══════════════════════════════════════════════════════"
echo ""

# ═══════════════════════════════════════════════════════
# System Status
# ═══════════════════════════════════════════════════════
echo "System Status:"
echo ""
echo "  Compositor:  $(ps -p ${COMPOSITOR_PID} > /dev/null && echo 'Running ✓' || echo 'Stopped ✗')"
[ -n "${GEARSTATE_PID}" ] && echo "  GearState:   $(ps -p ${GEARSTATE_PID} > /dev/null && echo 'Running ✓' || echo 'Stopped ✗')"
[ -n "${SPEEDOMETER_PID}" ] && echo "  Speedometer: $(ps -p ${SPEEDOMETER_PID} > /dev/null && echo 'Running ✓' || echo 'Stopped ✗')"
[ -n "${BATTERYMETER_PID}" ] && echo "  Battery:     $(ps -p ${BATTERYMETER_PID} > /dev/null && echo 'Running ✓' || echo 'Stopped ✗')"

echo ""
echo "═══════════════════════════════════════════════════════"
echo ""

# ═══════════════════════════════════════════════════════
# Create summary log
# ═══════════════════════════════════════════════════════
cat > ${LOG_DIR}/summary.log <<EOF
═══════════════════════════════════════════════════════
IC System Debug Summary
Generated: $(date)
═══════════════════════════════════════════════════════

ENVIRONMENT:
  Qt Path:         ${HOME}/Qt/5.15.2/gcc_64
  Runtime Dir:     ${XDG_RUNTIME_DIR}
  Wayland Display: ${WAYLAND_DISPLAY}
  Socket Path:     ${SOCKET_PATH}
  Log Directory:   ${LOG_DIR}

PROCESS IDs:
  Compositor:  ${COMPOSITOR_PID}
  GearState:   ${GEARSTATE_PID:-N/A}
  Speedometer: ${SPEEDOMETER_PID:-N/A}
  Battery:     ${BATTERYMETER_PID:-N/A}

APPLICATION PATHS:
  Compositor:  ${COMPOSITOR_PATH}
  GearState:   ${GEARSTATE_PATH}
  Speedometer: ${SPEEDOMETER_PATH}
  Battery:     ${BATTERYMETER_PATH}

LOG FILES:
  - compositor_stdout.log
  - compositor_stderr.log
  - gearstate_stdout.log (if started)
  - gearstate_stderr.log (if started)
  - speedometer_stdout.log (if started)
  - speedometer_stderr.log (if started)
  - batterymeter_stdout.log (if started)
  - batterymeter_stderr.log (if started)
  - socket_info.log
  - summary.log (this file)

═══════════════════════════════════════════════════════
EOF

# ═══════════════════════════════════════════════════════
# Instructions
# ═══════════════════════════════════════════════════════
echo "Debug Information:"
echo ""
echo "  All logs saved to: ${LOG_DIR}/"
echo ""
echo "  View logs:"
echo "    cat ${LOG_DIR}/compositor_stdout.log"
echo "    cat ${LOG_DIR}/compositor_stderr.log"
echo "    cat ${LOG_DIR}/gearstate_stdout.log"
echo "    cat ${LOG_DIR}/gearstate_stderr.log"
echo ""
echo "  Package logs for sharing:"
echo "    tar -czf ic_debug_logs.tar.gz ${LOG_DIR}/"
echo ""
echo "  Monitor logs in real-time:"
echo "    tail -f ${LOG_DIR}/*.log"
echo ""
echo "═══════════════════════════════════════════════════════"
echo ""
echo "System is running. Press Ctrl+C to stop and view summary."
echo ""

# ═══════════════════════════════════════════════════════
# Cleanup function
# ═══════════════════════════════════════════════════════
cleanup() {
    echo ""
    echo "═══════════════════════════════════════════════════════"
    echo "Stopping all processes..."
    echo "═══════════════════════════════════════════════════════"
    
    [ -n "${BATTERYMETER_PID}" ] && kill ${BATTERYMETER_PID} 2>/dev/null
    [ -n "${SPEEDOMETER_PID}" ] && kill ${SPEEDOMETER_PID} 2>/dev/null
    [ -n "${GEARSTATE_PID}" ] && kill ${GEARSTATE_PID} 2>/dev/null
    kill ${COMPOSITOR_PID} 2>/dev/null
    
    sleep 1
    
    pkill -9 -f GearState_app 2>/dev/null
    pkill -9 -f Speedometer_app 2>/dev/null
    pkill -9 -f BatteryMeter_app 2>/dev/null
    pkill -9 -f IC_Compositor 2>/dev/null
    
    echo ""
    echo "═══════════════════════════════════════════════════════"
    echo "Debug Session Summary"
    echo "═══════════════════════════════════════════════════════"
    echo ""
    cat ${LOG_DIR}/summary.log
    echo ""
    echo "═══════════════════════════════════════════════════════"
    echo "All logs available at: ${LOG_DIR}/"
    echo ""
    echo "To package logs for sharing:"
    echo "  tar -czf ic_debug_logs_$(date +%Y%m%d_%H%M%S).tar.gz ${LOG_DIR}/"
    echo ""
    echo "Key files to check:"
    echo "  - compositor_stdout.log (main compositor output)"
    echo "  - compositor_stderr.log (compositor errors)"
    echo "  - gearstate_stdout.log (client app output)"
    echo "  - gearstate_stderr.log (client app errors)"
    echo "═══════════════════════════════════════════════════════"
    
    exit 0
}

# Trap signals
trap cleanup SIGINT SIGTERM

# Wait for compositor
wait ${COMPOSITOR_PID}

# If compositor exits, cleanup
cleanup
