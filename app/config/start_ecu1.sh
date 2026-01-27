#!/bin/bash

# ECU1 VehicleControlECU 시작 스크립트
# 서비스 제공자 + 라우팅 매니저

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
VEHICLE_CONTROL_DIR="${PROJECT_ROOT}/VehicleControlECU"

echo "=========================================="
echo "ECU1 VehicleControlECU 시작"
echo "=========================================="
echo "Project Root: ${PROJECT_ROOT}"
echo ""

# 1단계: 완전 클린업
echo "[1/4] Cleaning up processes..."
killall -9 VehicleControlECU 2>/dev/null
pkill -9 -f vsomeip 2>/dev/null
sudo rm -rf /tmp/vsomeip-* 2>/dev/null
sleep 1
echo "✓ Cleanup complete"
echo ""

# 2단계: 네트워크 확인
echo "[2/4] Checking network configuration..."
IP_ADDR=$(ip addr show eth0 2>/dev/null | grep "inet " | awk '{print $2}' | cut -d'/' -f1)
if [ "$IP_ADDR" != "192.168.1.100" ]; then
    echo "⚠ Warning: Setting up network..."
    sudo nmcli device set eth0 managed no 2>/dev/null
    sudo ip link set eth0 up
    sudo ip addr flush dev eth0
    sudo ip addr add 192.168.1.100/24 dev eth0
    echo "✓ IP configured: 192.168.1.100"
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

# ECU2 연결 확인 (선택사항)
echo -n "⏳ Checking connection to ECU2 (192.168.1.101)... "
if ping -c 1 -W 2 192.168.1.101 &> /dev/null; then
    echo "✓ Connected"
else
    echo "⚠ Not reachable (ECU2 may not be ready yet)"
fi
echo ""

# 3단계: 빌드 확인
echo "[3/4] Checking VehicleControlECU build..."
if [ ! -f "${VEHICLE_CONTROL_DIR}/build/VehicleControlECU" ]; then
    echo "✗ VehicleControlECU not built!"
    echo "Run: cd ${VEHICLE_CONTROL_DIR} && ./build.sh"
    exit 1
fi
echo "✓ Build found"
echo ""

# 4단계: VehicleControlECU 실행
echo "[4/4] Starting VehicleControlECU..."
cd "${VEHICLE_CONTROL_DIR}"

export LD_LIBRARY_PATH=/usr/local/lib:${LD_LIBRARY_PATH}

# run.sh가 있으면 사용, 없으면 직접 실행
if [ -f "run.sh" ]; then
    ./run.sh
else
    export VSOMEIP_CONFIGURATION="${VEHICLE_CONTROL_DIR}/config/vsomeip_ecu1.json"
    export COMMONAPI_CONFIG="${VEHICLE_CONTROL_DIR}/config/commonapi_ecu1.ini"
    ./build/VehicleControlECU
fi