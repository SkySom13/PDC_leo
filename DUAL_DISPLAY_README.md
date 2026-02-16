# Dual Display Configuration for Automotive Infotainment System

## Overview

This document describes the complete configuration for running a dual-display automotive infotainment system on NVIDIA Jetson Orin Nano using Wayland, Weston compositor, and nested Qt Wayland compositors.

## System Architecture

### 3-Tier Wayland Architecture

```
┌─────────────────────────────────────────────────────────────┐
│ Tier 1: Weston Root Compositor (wayland-0)                  │
│ - Backend: DRM                                               │
│ - Shell: kiosk-shell                                         │
│ - Socket: /run/wayland-0                                     │
├──────────────────────┬───────────────────────────────────────┤
│  DP-2 (1920x1080)    │  DP-3 (1024x600)                      │
│  Head Unit Display   │  Instrument Cluster Display           │
│  Routes: HeadUnitApp │  Routes: IC_Compositor                │
└──────────────────────┴───────────────────────────────────────┘
           ▼                            ▼
┌──────────────────────┐    ┌───────────────────────────────┐
│ Tier 2: HU Compositor│    │ Tier 2: IC Compositor         │
│ - HU_MainApp_Comp    │    │ - IC_Compositor               │
│ - Socket: wayland-3  │    │ - Socket: wayland-2           │
│ - App-ID: HeadUnitApp│    │ - App-ID: IC_Compositor       │
└──────────────────────┘    └───────────────────────────────┘
           ▼                            ▼
┌──────────────────────┐    ┌───────────────────────────────┐
│ Tier 3: HU Apps      │    │ Tier 3: IC Apps               │
│ - GearApp            │    │ - Speedometer_app             │
│ - MediaApp           │    │ - BatteryMeter_app            │
│ - AmbientApp         │    │ - GearState_app               │
│ - HomeScreenApp      │    │                               │
└──────────────────────┘    └───────────────────────────────┘
```

## Hardware Configuration

- **Platform**: NVIDIA Jetson Orin Nano DevKit
- **Display 1 (DP-2)**: 1920x1080@60Hz - Head Unit
- **Display 2 (DP-3)**: 1024x600@59.85Hz - Instrument Cluster
- **Connection**: DisplayPort via MST hub/splitter

## Software Stack

- **OS**: Yocto Linux (custom embedded build)
- **Weston**: System Wayland compositor (socket-activated)
- **Qt**: 5.15.2 with Wayland support
- **Compositors**: Qt-based nested Wayland compositors
- **IPC**: vsomeip for application communication

---

## Configuration Files

### 1. Weston Configuration

**File**: `/etc/xdg/weston/weston.ini`

```ini
[core]
backend=drm-backend.so
shell=kiosk-shell.so
require-input=false

# Display 1 - Head Unit (DP-2)
[output]
name=DP-2
app-ids=HeadUnitApp

# Display 2 - Instrument Cluster (DP-3)
[output]
name=DP-3
app-ids=IC_Compositor

[shell]
background-color=0xff000000
locking=false

[libinput]
enable-tap=true

[keyboard]
keymap_layout=us
```

**Key Points**:
- Uses `kiosk-shell.so` for app-id based window routing
- `app-ids` parameter routes specific applications to specific displays
- Auto-detects display modes (no explicit mode specification needed)

---

### 2. HU Compositor Service Override

**File**: `/etc/systemd/system/hu-mainapp-compositor.service.d/wayland-display.conf`

```ini
[Service]
Environment="WAYLAND_DISPLAY=/run/wayland-0"
Environment="XDG_RUNTIME_DIR=/run/user/1000"
Environment="QT_QPA_PLATFORM=wayland"
Environment="QT_WAYLAND_DISABLE_WINDOWDECORATION=1"
Environment="QSG_RENDER_LOOP=basic"
Environment="QT_QUICK_BACKEND=software"
Environment="QT_QPA_FONTDIR=/usr/share/fonts"
Environment="FONTCONFIG_FILE=/etc/fonts/fonts.conf"
```

**Original Service** (`/usr/lib/systemd/system/hu-mainapp-compositor.service`):
```ini
[Unit]
Description=HU Main App Wayland Compositor
After=weston.service vsomeip-routing-manager.service
Requires=weston.service vsomeip-routing-manager.service

[Service]
Type=simple
User=weston
Environment="XDG_RUNTIME_DIR=/run/user/1000"
Environment="WAYLAND_DISPLAY=wayland-1"
Environment="QT_QPA_PLATFORM=wayland"
Environment="QT_WAYLAND_DISABLE_WINDOWDECORATION=1"
Environment="QSG_RENDER_LOOP=basic"
Environment="QT_QUICK_BACKEND=software"
Environment="QT_QPA_FONTDIR=/usr/share/fonts"
Environment="QT_LOGGING_RULES=qt.qpa.fonts=true"
Environment="FONTCONFIG_FILE=/etc/fonts/fonts.conf"
ExecStart=/usr/bin/HU_MainApp_Compositor
Restart=always
RestartSec=3

[Install]
WantedBy=multi-user.target
```

**Change**: `WAYLAND_DISPLAY` changed from `wayland-1` to `/run/wayland-0` (full path to Weston's socket)

---

### 3. IC Compositor Service Override

**File**: `/etc/systemd/system/ic-compositor.service.d/wayland-display.conf`

```ini
[Service]
Environment="WAYLAND_DISPLAY=/run/wayland-0"
Environment="XDG_RUNTIME_DIR=/run/user/1000"
Environment="QT_QPA_PLATFORM=wayland"
Environment="QT_WAYLAND_DISABLE_WINDOWDECORATION=1"
Environment="QSG_RENDER_LOOP=basic"
Environment="QT_QUICK_BACKEND=software"
Environment="QT_QPA_FONTDIR=/usr/share/fonts"
Environment="FONTCONFIG_FILE=/etc/fonts/fonts.conf"
```

**Original Service** (`/usr/lib/systemd/system/ic-compositor.service`):
```ini
[Unit]
Description=IC Compositor - Instrument Cluster Wayland Compositor
After=weston.service
Requires=weston.service

[Service]
Type=simple
User=weston
Environment="XDG_RUNTIME_DIR=/run/user/1000"
Environment="WAYLAND_DISPLAY=wayland-1"
Environment="QT_QPA_PLATFORM=wayland"
Environment="QT_WAYLAND_DISABLE_WINDOWDECORATION=1"
Environment="QSG_RENDER_LOOP=basic"
Environment="QT_QUICK_BACKEND=software"
Environment="QT_QPA_FONTDIR=/usr/share/fonts"
Environment="FONTCONFIG_FILE=/etc/fonts/fonts.conf"
ExecStart=/usr/bin/IC_Compositor
Restart=on-failure
RestartSec=5

[Install]
WantedBy=graphical.target
```

**Change**: `WAYLAND_DISPLAY` changed from `wayland-1` to `/run/wayland-0`

---

## Application Services

Application services remain unchanged. They connect to nested compositor sockets:

- **HU Apps** (`GearApp`, `MediaApp`, `AmbientApp`, `HomeScreenApp`):
  - Connect to: `WAYLAND_DISPLAY=wayland-3`
  - Created by: `HU_MainApp_Compositor`

- **IC Apps** (`Speedometer_app`, `BatteryMeter_app`, `GearState_app`):
  - Connect to: `WAYLAND_DISPLAY=wayland-2`
  - Created by: `IC_Compositor`

**Example** (`gearapp.service`):
```ini
[Unit]
Description=Gear Selection Application
Requires=hu-mainapp-compositor.service vsomeip-routing-manager.service
After=hu-mainapp-compositor.service vsomeip-routing-manager.service

[Service]
Type=simple
User=weston
Environment="WAYLAND_DISPLAY=wayland-3"
Environment="XDG_RUNTIME_DIR=/run/user/1000"
Environment="QT_QPA_PLATFORM=wayland"
# ... other environment variables ...
ExecStart=/usr/bin/GearApp

[Install]
WantedBy=multi-user.target
```

---

## Wayland Socket Hierarchy

```
/run/wayland-0           → Weston root compositor (socket-activated)
                           Created by: weston.socket
                           Clients: HU_MainApp_Compositor, IC_Compositor

/run/user/1000/wayland-2 → IC nested compositor socket
                           Created by: IC_Compositor
                           Clients: Speedometer_app, BatteryMeter_app, GearState_app

/run/user/1000/wayland-3 → HU nested compositor socket
                           Created by: HU_MainApp_Compositor
                           Clients: GearApp, MediaApp, AmbientApp, HomeScreenApp
```

---

## Critical Implementation Requirements

### 1. App-ID Configuration in Compositor Code

For kiosk-shell routing to work, compositors **MUST** set their application name in C++ code:

**HU_MainApp_Compositor/main.cpp**:
```cpp
#include <QGuiApplication>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // CRITICAL: Set app-id for kiosk-shell routing
    app.setApplicationName("HeadUnitApp");

    // ... rest of compositor initialization ...

    return app.exec();
}
```

**IC_Compositor/main.cpp**:
```cpp
#include <QGuiApplication>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    // CRITICAL: Set app-id for kiosk-shell routing
    app.setApplicationName("IC_Compositor");

    // ... rest of compositor initialization ...

    return app.exec();
}
```

⚠️ **Important**: Environment variables **cannot** be used to set Qt application names. This must be done in code.

---

## Installation & Deployment

### Step 1: Backup Original Configuration

```bash
# Create backup directory
mkdir -p /root/weston_backup_$(date +%Y%m%d_%H%M%S)

# Backup weston.ini
cp /etc/xdg/weston/weston.ini /root/weston_backup_*/
```

### Step 2: Update Weston Configuration

```bash
cat > /etc/xdg/weston/weston.ini << 'EOF'
[core]
backend=drm-backend.so
shell=kiosk-shell.so
require-input=false

[output]
name=DP-2
app-ids=HeadUnitApp

[output]
name=DP-3
app-ids=IC_Compositor

[shell]
background-color=0xff000000
locking=false

[libinput]
enable-tap=true

[keyboard]
keymap_layout=us
EOF
```

### Step 3: Create Compositor Service Overrides

```bash
# HU Compositor override
mkdir -p /etc/systemd/system/hu-mainapp-compositor.service.d

cat > /etc/systemd/system/hu-mainapp-compositor.service.d/wayland-display.conf << 'EOF'
[Service]
Environment="WAYLAND_DISPLAY=/run/wayland-0"
Environment="XDG_RUNTIME_DIR=/run/user/1000"
Environment="QT_QPA_PLATFORM=wayland"
Environment="QT_WAYLAND_DISABLE_WINDOWDECORATION=1"
Environment="QSG_RENDER_LOOP=basic"
Environment="QT_QUICK_BACKEND=software"
Environment="QT_QPA_FONTDIR=/usr/share/fonts"
Environment="FONTCONFIG_FILE=/etc/fonts/fonts.conf"
EOF

# IC Compositor override
mkdir -p /etc/systemd/system/ic-compositor.service.d

cat > /etc/systemd/system/ic-compositor.service.d/wayland-display.conf << 'EOF'
[Service]
Environment="WAYLAND_DISPLAY=/run/wayland-0"
Environment="XDG_RUNTIME_DIR=/run/user/1000"
Environment="QT_QPA_PLATFORM=wayland"
Environment="QT_WAYLAND_DISABLE_WINDOWDECORATION=1"
Environment="QSG_RENDER_LOOP=basic"
Environment="QT_QUICK_BACKEND=software"
Environment="QT_QPA_FONTDIR=/usr/share/fonts"
Environment="FONTCONFIG_FILE=/etc/fonts/fonts.conf"
EOF
```

### Step 4: Reload and Restart Services

```bash
# Reload systemd configuration
systemctl daemon-reload

# Restart Weston (will auto-restart via socket)
killall -9 weston
sleep 3

# Restart compositors
systemctl restart hu-mainapp-compositor.service
systemctl restart ic-compositor.service

# Restart applications
systemctl restart gearapp.service
systemctl restart mediaapp.service
systemctl restart ambientapp.service
systemctl restart homescreenapp.service
systemctl restart speedometer-app.service
systemctl restart batterymeter-app.service
systemctl restart gearstate-app.service
```

### Step 5: Verification

```bash
# Check Weston is running on both displays
systemctl status weston.service

# Check compositors are running
systemctl status hu-mainapp-compositor.service
systemctl status ic-compositor.service

# Check applications are running
ps aux | grep -E "GearApp|MediaApp|Speedometer|Battery"

# Verify socket creation
ls -la /run/wayland-0
ls -la /run/user/1000/wayland-*

# Check display detection in journal
journalctl -u weston.service --since "5 minutes ago" | grep -i "DP-"
```

**Expected Output**:
- Weston: `active (running)`
- Both compositors: `active (running)`
- All application processes visible
- Sockets: `/run/wayland-0`, `/run/user/1000/wayland-2`, `/run/user/1000/wayland-3`
- Journal shows both DP-2 and DP-3 detected and enabled

---

## Troubleshooting

### Issue 1: Compositors Crashing with "Failed to create wl_display"

**Symptom**:
```
HU_MainApp_Compositor: Failed to create wl_display (No such file or directory)
qt.qpa.plugin: Could not load the Qt platform plugin "wayland"
```

**Cause**: Compositor trying to connect to wrong socket location

**Solution**: Ensure `WAYLAND_DISPLAY=/run/wayland-0` (full path) in service override

### Issue 2: Black Screens on Both Displays

**Symptom**: Both displays show black, no compositor content

**Possible Causes**:
1. Compositors not setting app-id in code
2. Compositors not connecting to wayland-0
3. Kiosk-shell not routing windows

**Solution**:
```bash
# Check if compositors are running
systemctl status hu-mainapp-compositor.service
systemctl status ic-compositor.service

# Check compositor logs
journalctl -u hu-mainapp-compositor.service -n 50
journalctl -u ic-compositor.service -n 50

# Verify app-id is set in compositor source code
# Must have: app.setApplicationName("HeadUnitApp") or app.setApplicationName("IC_Compositor")
```

### Issue 3: systemctl restart weston.service Hangs

**Symptom**: Command never returns

**Cause**: Socket-activated Weston service conflict

**Solution**: Don't restart service directly, kill process instead:
```bash
killall -9 weston
# weston.socket will auto-restart it
sleep 3
systemctl status weston.service
```

### Issue 4: Apps Not Appearing on Compositors

**Symptom**: Compositors running but apps don't show

**Cause**: Apps connecting to wrong socket

**Solution**: Verify app services use correct WAYLAND_DISPLAY:
- HU apps → `wayland-3`
- IC apps → `wayland-2`

```bash
# Check app service configuration
systemctl cat gearapp.service | grep WAYLAND_DISPLAY
systemctl cat speedometer-app.service | grep WAYLAND_DISPLAY

# Restart apps
systemctl restart gearapp.service
systemctl restart speedometer-app.service
```

---

## Rollback Procedure

If dual display configuration causes issues:

```bash
# 1. Stop compositors
systemctl stop hu-mainapp-compositor.service
systemctl stop ic-compositor.service

# 2. Restore original weston.ini
BACKUP=$(ls -t /root/weston_backup_*/weston.ini | head -1)
cp "$BACKUP" /etc/xdg/weston/weston.ini

# 3. Remove service overrides
rm -rf /etc/systemd/system/hu-mainapp-compositor.service.d
rm -rf /etc/systemd/system/ic-compositor.service.d

# 4. Reload systemd
systemctl daemon-reload

# 5. Restart Weston
killall -9 weston
sleep 3

# 6. Restart compositors (will use original config)
systemctl start hu-mainapp-compositor.service
systemctl start ic-compositor.service

# 7. Restart applications
systemctl restart gearapp.service
systemctl restart mediaapp.service
# ... etc
```

---

## Key Concepts

### Kiosk-Shell vs Desktop-Shell

**kiosk-shell**:
- Routes windows to specific outputs based on app-id
- No window decorations, panels, or user interaction
- Perfect for fixed-layout automotive systems
- Requires app-id to be set in application code

**desktop-shell**:
- Traditional desktop environment
- Window decorations, panels, workspace management
- Windows not automatically routed to specific displays

### Socket Activation

Weston uses systemd socket activation:
- `weston.socket` creates `/run/wayland-0` before Weston starts
- First client connection triggers Weston startup
- Enables zero-memory idle state
- Requires special handling for restarts (kill process, don't use systemctl restart)

### Nested Compositor Pattern

Nested compositors act as both:
1. **Wayland client** (to parent Weston compositor)
2. **Wayland server** (for their own applications)

This enables:
- Independent display management per screen
- Different rendering pipelines
- Isolation between HU and IC subsystems
- Compositor-specific window management

---

## Performance Considerations

### Software Rendering

Current configuration uses software rendering:
```
QSG_RENDER_LOOP=basic
QT_QUICK_BACKEND=software
```

**Pros**:
- Maximum compatibility
- Predictable behavior
- No GPU contention

**Cons**:
- Higher CPU usage
- Lower frame rates for complex UIs

### Hardware Acceleration (Optional)

For GPU-accelerated rendering:

```ini
# Remove or change these environment variables:
# Environment="QSG_RENDER_LOOP=basic"      → Remove
# Environment="QT_QUICK_BACKEND=software"  → Remove

# Add:
Environment="QT_QPA_EGLFS_INTEGRATION=eglfs_kms"
Environment="QSG_RENDER_LOOP=threaded"
```

⚠️ **Note**: Requires testing for stability with nested compositors and dual displays.

---

## Testing & Validation

### Display Detection Test

```bash
# Check physical display connections
cat /sys/class/drm/card0-DP-2/status  # Should show "connected"
cat /sys/class/drm/card0-DP-3/status  # Should show "connected"

# Check Weston detected both
journalctl -u weston.service --since boot | grep -i "output.*DP-"
```

### Window Routing Test

```bash
# Stop all apps
systemctl stop gearapp.service mediaapp.service speedometer-app.service

# Start one HU app
systemctl start gearapp.service

# Verify it appears on DP-2 (HU display), not DP-3

# Start one IC app
systemctl start speedometer-app.service

# Verify it appears on DP-3 (IC display), not DP-2
```

### Socket Creation Test

```bash
# Verify all sockets exist and have correct permissions
ls -la /run/wayland-0                    # Owner: weston, Group: wayland
ls -la /run/user/1000/wayland-2          # Created by IC_Compositor
ls -la /run/user/1000/wayland-3          # Created by HU_MainApp_Compositor

# Check socket accessibility
sudo -u weston test -S /run/wayland-0 && echo "OK" || echo "FAIL"
sudo -u weston test -S /run/user/1000/wayland-2 && echo "OK" || echo "FAIL"
sudo -u weston test -S /run/user/1000/wayland-3 && echo "OK" || echo "FAIL"
```

---

## Future Improvements

### 1. Hardware Acceleration
Enable GPU rendering for better performance and lower CPU usage.

### 2. Dynamic Display Configuration
Support for hotplug detection and dynamic display changes.

### 3. Compositor Failover
Implement automatic restart and recovery for crashed compositors.

### 4. Logging & Monitoring
Centralized logging for all compositor and application events.

### 5. Performance Metrics
Real-time monitoring of frame rates, latency, and resource usage.

---

## References

- **Weston Documentation**: https://wayland.freedesktop.org/
- **Qt Wayland Compositor**: https://doc.qt.io/qt-5/qtwaylandcompositor-index.html
- **systemd Socket Activation**: https://www.freedesktop.org/software/systemd/man/systemd.socket.html
- **Kiosk Shell**: https://gitlab.freedesktop.org/wayland/weston/-/tree/main/kiosk-shell

---

## Author & Maintenance

**Configuration Date**: February 16, 2026  
**Platform**: NVIDIA Jetson Orin Nano DevKit  
**Yocto Build Date**: March 9, 2018 (embedded timestamp)

**Maintained By**: [Your Name/Team]  
**Last Updated**: February 16, 2026

---

## License

[Your License Here]

---

## Appendix: Complete File Tree

```
/etc/xdg/weston/
└── weston.ini                                      [MODIFIED]

/etc/systemd/system/
├── hu-mainapp-compositor.service.d/
│   └── wayland-display.conf                       [NEW]
└── ic-compositor.service.d/
    └── wayland-display.conf                       [NEW]

/usr/lib/systemd/system/
├── weston.service                                 [ORIGINAL - unchanged]
├── weston.socket                                  [ORIGINAL - unchanged]
├── hu-mainapp-compositor.service                  [ORIGINAL - unchanged]
├── ic-compositor.service                          [ORIGINAL - unchanged]
├── gearapp.service                                [ORIGINAL - unchanged]
├── mediaapp.service                               [ORIGINAL - unchanged]
├── ambientapp.service                             [ORIGINAL - unchanged]
├── homescreenapp.service                          [ORIGINAL - unchanged]
├── speedometer-app.service                        [ORIGINAL - unchanged]
├── batterymeter-app.service                       [ORIGINAL - unchanged]
└── gearstate-app.service                          [ORIGINAL - unchanged]

/run/
└── wayland-0                                      [Runtime socket - Weston]

/run/user/1000/
├── wayland-2                                      [Runtime socket - IC_Compositor]
└── wayland-3                                      [Runtime socket - HU_MainApp_Compositor]

/root/
└── weston_backup_YYYYMMDD_HHMMSS/
    └── weston.ini                                 [BACKUP]
```

---

**END OF DOCUMENTATION**
