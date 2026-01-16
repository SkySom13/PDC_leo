#!/bin/bash

# ECU2 전체 시스템 시작 스크립트 (GearApp 라우팅 매니저 방식)
# 별도 routingmanagerd 불필요 - GearApp이 라우팅 매니저 역할

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

echo "=========================================="
echo "ECU2 전체 시스템 시작 (GearApp 라우팅)"
echo "=========================================="
echo "Project Root: ${PROJECT_ROOT}"
echo ""

# 1단계: 완전 클린업
echo "[1/5] Cleaning up all processes..."
killall -9 GearApp AmbientApp IC_app MediaApp 2>/dev/null
pkill -9 -f vsomeip 2>/dev/null
sudo rm -rf /tmp/vsomeip-* 2>/dev/null
sleep 1
echo "✓ Cleanup complete"
echo ""

# 2단계: 네트워크 확인
echo "[2/5] Checking network configuration..."
IP_ADDR=$(ip addr show eth0 2>/dev/null | grep "inet " | awk '{print $2}' | cut -d'/' -f1)
if [ "$IP_ADDR" != "192.168.1.101" ]; then
    echo "⚠ Warning: Setting up network..."
    sudo nmcli device set eth0 managed no 2>/dev/null
    sudo ip link set eth0 up
    sudo ip addr flush dev eth0
    sudo ip addr add 192.168.1.101/24 dev eth0
    echo "✓ IP configured: 192.168.1.101"
else
    echo "✓ IP Address: ${IP_ADDR}"
fi

MULTICAST_ROUTE=$(ip route | grep "224.0.0.0/4")
if [ -z "$MULTICAST_ROUTE" ]; then
    echo "⚠ Adding multicast route..."
    sudo ip route add 224.0.0.0/4 dev eth0
    echo "✓ Multicast route added"
else
    echo "✓ Multicast route: OK"
fi

# ECU1 연결 확인
echo -n "⏳ Checking connection to ECU1 (192.168.1.100)... "
if ping -c 1 -W 2 192.168.1.100 &> /dev/null; then
    echo "✓ Connected"
else
    echo "✗ Failed"
    echo ""
    echo "❌ Cannot reach ECU1!"
    echo "Make sure:"
    echo "  1. ECU1 is powered on"
    echo "  2. Ethernet cable is connected"
    echo "  3. ECU1 IP is 192.168.1.100"
    exit 1
fi
echo ""

# 3단계: GearApp 시작 (라우팅 매니저 역할)
echo "[3/5] Starting GearApp (Routing Manager)..."
cd "${PROJECT_ROOT}/app/GearApp"
if [ ! -f "build/GearApp" ]; then
    echo "✗ GearApp not built! Run: ./build.sh"
    exit 1
fi
./run.sh &> /tmp/gearapp.log &
GEARAPP_PID=$!
echo "✓ GearApp started (PID: ${GEARAPP_PID})"
echo "  - Role: [Host] Routing Manager + Client"
sleep 3

# 라우팅 매니저 확인
if [ ! -e /tmp/vsomeip-0 ]; then
    echo "✗ Routing Manager failed to start!"
    echo "Check log: tail /tmp/gearapp.log"
    exit 1
fi
echo "✓ Routing Manager ready (/tmp/vsomeip-0)"
echo ""

# 4단계: AmbientApp 시작
echo "[4/5] Starting AmbientApp..."
cd "${PROJECT_ROOT}/app/AmbientApp"
if [ ! -f "build/AmbientApp" ]; then
    echo "✗ AmbientApp not built! Run: ./build.sh"
    exit 1
fi
./run.sh &> /tmp/ambientapp.log &
AMBIENTAPP_PID=$!
echo "✓ AmbientApp started (PID: ${AMBIENTAPP_PID})"
sleep 2
echo ""

# 5단계: IC_app 시작
echo "[5/5] Starting IC_app..."
cd "${PROJECT_ROOT}/app/IC_app"
if [ ! -f "build/IC_app" ]; then
    echo "⚠ IC_app not built, skipping..."
else
    ./run.sh &> /tmp/ic_app.log &
    IC_APP_PID=$!
    echo "✓ IC_app started (PID: ${IC_APP_PID})"
fi
echo ""

# 완료
echo "=========================================="
echo "✅ ECU2 시스템 시작 완료!"
echo "=========================================="
echo ""
echo "실행 중인 프로세스:"
echo "  - GearApp (RM):    PID ${GEARAPP_PID}"
echo "  - AmbientApp:      PID ${AMBIENTAPP_PID}"
if [ ! -z "$IC_APP_PID" ]; then
    echo "  - IC_app:          PID ${IC_APP_PID}"
fi
echo ""
echo "로그 파일:"
echo "  - GearApp:         /tmp/gearapp.log"
echo "  - AmbientApp:      /tmp/ambientapp.log"
if [ ! -z "$IC_APP_PID" ]; then
    echo "  - IC_app:          /tmp/ic_app.log"
fi
echo ""
echo "실시간 로그 확인:"
echo "  tail -f /tmp/gearapp.log"
echo "  tail -f /tmp/ambientapp.log"
echo ""
echo "전체 종료:"
echo "  killall -9 GearApp AmbientApp IC_app MediaApp"
echo "  sudo rm -rf /tmp/vsomeip-*"
echo ""