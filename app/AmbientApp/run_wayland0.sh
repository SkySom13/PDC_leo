#!/bin/bash

# ════════════════════════════════════════════════════════════
# AmbientApp 실행 스크립트 (Direct to Weston - wayland-0)
# IVI-Shell Single Compositor Architecture
# ════════════════════════════════════════════════════════════

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_NAME="AmbientApp"

# vsomeip 환경변수
export VSOMEIP_APPLICATION_NAME="${APP_NAME}"
export VSOMEIP_CONFIGURATION="${SCRIPT_DIR}/vsomeip_ambient.json"
export COMMONAPI_CONFIG="${SCRIPT_DIR}/commonapi_ambient.ini"

# 라이브러리 경로 설정
if [ -z "$DEPLOY_PREFIX" ]; then
    export DEPLOY_PREFIX="${HOME}/leo/DES_Head-Unit/install_folder"
fi
export LD_LIBRARY_PATH="${DEPLOY_PREFIX}/lib:${LD_LIBRARY_PATH}"

# ════════════════════════════════════════════════════════════
# Wayland 설정 - CRITICAL CHANGE
# ════════════════════════════════════════════════════════════
# Before: WAYLAND_DISPLAY=wayland-1 (HU_MainApp_Compositor)
# After:  WAYLAND_DISPLAY=wayland-0 (Direct to Weston)
export WAYLAND_DISPLAY=wayland-0

# Qt Wayland 설정
export QT_QPA_PLATFORM=wayland
export QT_WAYLAND_DISABLE_WINDOWDECORATION=1
export QT_PLUGIN_PATH=/usr/lib/aarch64-linux-gnu/qt5/plugins:${QT_PLUGIN_PATH}

# IVI Surface ID (AmbientApp = 3000)

# Qt 렌더링 최적화
export QT_QUICK_BACKEND=software  # Critical for performance

echo "════════════════════════════════════════════════════════════"
echo "Running ${APP_NAME} - Direct to Weston (IVI-Shell)"
echo "════════════════════════════════════════════════════════════"
echo "WAYLAND_DISPLAY: ${WAYLAND_DISPLAY}"
echo "VSOMEIP_CONFIGURATION: ${VSOMEIP_CONFIGURATION}"
echo "COMMONAPI_CONFIG: ${COMMONAPI_CONFIG}"
echo "DEPLOY_PREFIX: ${DEPLOY_PREFIX}"
echo "LD_LIBRARY_PATH: ${LD_LIBRARY_PATH}"
echo "════════════════════════════════════════════════════════════"

# 실행 파일 경로
if [ -f "${SCRIPT_DIR}/build/${APP_NAME}" ]; then
    EXEC_PATH="${SCRIPT_DIR}/build/${APP_NAME}"
elif [ -f "${SCRIPT_DIR}/${APP_NAME}" ]; then
    EXEC_PATH="${SCRIPT_DIR}/${APP_NAME}"
else
    echo "❌ Error: ${APP_NAME} executable not found!"
    echo "   Searched in:"
    echo "   - ${SCRIPT_DIR}/build/${APP_NAME}"
    echo "   - ${SCRIPT_DIR}/${APP_NAME}"
    exit 1
fi

echo "Executable: ${EXEC_PATH}"
echo ""

# 실행
exec "${EXEC_PATH}"
