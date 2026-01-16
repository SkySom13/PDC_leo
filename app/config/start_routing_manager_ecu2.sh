#!/bin/bash

# ECU2 Routing Manager 시작 스크립트
# 모든 HU 앱(GearApp, AmbientApp, MediaApp, IC_app)이 공유하는 라우팅 매니저

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="${SCRIPT_DIR}/routing_manager_ecu2.json"

echo "=========================================="
echo "Starting ECU2 Routing Manager"
echo "=========================================="
echo "Config: ${CONFIG_FILE}"
echo ""

# 기존 vsomeip 프로세스 정리
echo "[1/4] Cleaning up existing vsomeip processes..."
killall -9 routingmanagerd 2>/dev/null
sudo rm -rf /tmp/vsomeip-* 2>/dev/null
echo "✓ Cleanup complete"
echo ""

# 네트워크 확인
echo "[2/4] Checking network configuration..."
IP_ADDR=$(ip addr show eth0 2>/dev/null | grep "inet " | awk '{print $2}' | cut -d'/' -f1)
if [ -z "$IP_ADDR" ]; then
    echo "⚠ Warning: eth0 not configured"
    echo "Run: sudo ip addr add 192.168.1.101/24 dev eth0"
else
    echo "✓ IP Address: ${IP_ADDR}"
fi

MULTICAST_ROUTE=$(ip route | grep "224.0.0.0/4")
if [ -z "$MULTICAST_ROUTE" ]; then
    echo "⚠ Warning: Multicast route not configured"
    echo "Run: sudo ip route add 224.0.0.0/4 dev eth0"
else
    echo "✓ Multicast route: ${MULTICAST_ROUTE}"
fi
echo ""

# 라우팅 매니저 실행
echo "[3/4] Starting Routing Manager daemon..."
export VSOMEIP_CONFIGURATION="${CONFIG_FILE}"
export VSOMEIP_APPLICATION_NAME="routingmanagerd"
export LD_LIBRARY_PATH=/usr/local/lib:${LD_LIBRARY_PATH}

# routingmanagerd 실행 (백그라운드)
if command -v routingmanagerd &> /dev/null; then
    routingmanagerd &
    RM_PID=$!
    echo "✓ Routing Manager started (PID: ${RM_PID})"
else
    echo "✗ Error: routingmanagerd not found in PATH"
    echo ""
    echo "Please install vsomeip or use vsomeipd:"
    echo "  # Alternative: use vsomeipd"
    echo "  VSOMEIP_CONFIGURATION=${CONFIG_FILE} vsomeipd &"
    exit 1
fi
echo ""

# 초기화 대기
echo "[4/4] Waiting for routing manager to initialize..."
sleep 2
echo "✓ Routing Manager ready"
echo ""

# 상태 확인
if [ -e /tmp/vsomeip-0 ]; then
    echo "=========================================="
    echo "✅ ECU2 Routing Manager is RUNNING"
    echo "=========================================="
    echo "Socket: /tmp/vsomeip-0"
    echo "PID: ${RM_PID}"
    echo ""
    echo "Now you can start applications:"
    echo "  1. GearApp"
    echo "  2. AmbientApp"
    echo "  3. IC_app"
    echo "  4. MediaApp"
    echo ""
    echo "To stop: kill ${RM_PID}"
else
    echo "=========================================="
    echo "❌ Routing Manager FAILED to start"
    echo "=========================================="
    echo "Check logs above for errors"
    exit 1
fi
