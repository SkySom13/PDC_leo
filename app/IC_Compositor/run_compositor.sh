#!/bin/bash

# Set environment for COMPOSITOR (runs on X11, NOT Wayland)
export LD_LIBRARY_PATH="${HOME}/Qt/5.15.2/gcc_64/lib:${LD_LIBRARY_PATH}"
export QT_QPA_PLATFORM=xcb  # Compositor runs on X11
export XDG_RUNTIME_DIR="/tmp/runtime-${USER}"

# Create runtime directory
mkdir -p ${XDG_RUNTIME_DIR}
chmod 0700 ${XDG_RUNTIME_DIR}

# Clean up any existing Wayland sockets
rm -f ${XDG_RUNTIME_DIR}/wayland-*

echo "═══════════════════════════════════════════════════════"
echo "Starting IC Compositor on X11..."
echo "Runtime Dir: ${XDG_RUNTIME_DIR}"
echo "Platform: XCB (X11)"
echo "═══════════════════════════════════════════════════════"

# Run compositor
cd build
./IC_Compositor
