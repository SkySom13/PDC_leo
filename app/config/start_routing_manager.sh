#!/bin/bash

# Start vsomeip Routing Manager
# This should be started ONCE before running HU or IC apps

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}/../.."

ROUTING_MANAGER_PATH="${PROJECT_ROOT}/deps/vsomeip/build/examples/routingmanagerd/routingmanagerd"

echo "========================================"
echo "Starting vsomeip Routing Manager"
echo "========================================"

# Check if routing manager exists
if [ ! -f "${ROUTING_MANAGER_PATH}" ]; then
    echo "✗ Routing manager not found at: ${ROUTING_MANAGER_PATH}"
    echo "Please build vsomeip first."
    exit 1
fi

# Check if routing manager is already running
if pgrep -x "routingmanagerd" > /dev/null; then
    echo "⚠️  Routing manager is already running!"
    ps aux | grep routingmanagerd | grep -v grep
    echo ""
    echo "To restart, first kill it:"
    echo "  killall -9 routingmanagerd"
    exit 1
fi

# Clean up old vsomeip sockets
rm -rf /tmp/vsomeip-* 2>/dev/null || true

# Set up environment
export LD_LIBRARY_PATH="${PROJECT_ROOT}/install_folder/lib:${LD_LIBRARY_PATH}"
export VSOMEIP_CONFIGURATION="${SCRIPT_DIR}/routing_manager_ecu2.json"

echo "Starting routing manager..."
echo "  Config: ${VSOMEIP_CONFIGURATION}"
echo "  Path: ${ROUTING_MANAGER_PATH}"
echo ""

# Start routing manager in background
"${ROUTING_MANAGER_PATH}" > /tmp/routingmanager.log 2>&1 &
ROUTING_PID=$!

sleep 2

# Check if it's running
if ! ps -p $ROUTING_PID > /dev/null 2>&1; then
    echo "✗ Routing manager failed to start"
    echo "Check log: /tmp/routingmanager.log"
    cat /tmp/routingmanager.log
    exit 1
fi

echo "✓ Routing manager started (PID: $ROUTING_PID)"
echo "  Socket: /tmp/vsomeip-0"
echo "  Log: /tmp/routingmanager.log"
echo ""
echo "You can now start HU apps and IC apps"
