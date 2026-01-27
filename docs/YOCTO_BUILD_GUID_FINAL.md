# Jetson Orin Nano - HU/IC Yocto Build Guide

**작성일**: 2026년 1월 27일  
**목표**: Jetson Orin Nano ECU2용 HU/IC 듀얼 시스템 Yocto 이미지 빌드  
**현재 검증 상태**: ✅ HU 4개 앱 + IC 3개 앱 완벽 동작 (듀얼 디스플레이 제외)

---

## 📋 목차

1. [시스템 아키텍처](#1-시스템-아키텍처)
2. [검증된 구성 요약](#2-검증된-구성-요약)
3. [Yocto Layer 구조](#3-yocto-layer-구조)
4. [Recipe 작성 가이드](#4-recipe-작성-가이드)
5. [Weston 설정](#5-weston-설정)
6. [systemd 서비스 구성](#6-systemd-서비스-구성)
7. [빌드 실행](#7-빌드-실행)
8. [배포 및 검증](#8-배포-및-검증)
9. [듀얼 디스플레이 전환 (향후)](#9-듀얼-디스플레이-전환-향후)

---

## 1. 시스템 아키텍처

### 1.1 현재 검증된 아키텍처 (단일 디스플레이)

```
Hardware: Jetson Orin Nano (ARM64 aarch64)
Display: DP-1 @ 1920x1080 (Single DisplayPort)

┌─────────────────────────────────────────────────────────────────┐
│ Weston (wayland-0) @ DP-1 1920x1080                            │
│ Backend: DRM (nvidia_drm modeset=1)                             │
│ Shell: desktop-shell.so (panel-position=none)                   │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│  ┌────────────────────────────────────────────────────────┐    │
│  │ HU_MainApp_Compositor (wayland-1) 1024x600             │    │
│  │ WaylandCompositor (Nested, Kiosk Mode)                 │    │
│  ├────────────────────────────────────────────────────────┤    │
│  │  ┌───────────┐ ┌─────────────────────────────────┐    │    │
│  │  │ GearApp   │ │ MediaApp / AmbientApp /         │    │    │
│  │  │ 130x1000  │ │ HomeScreenApp (1790x1000)       │    │    │
│  │  │ (Panel)   │ │ (Main Area, app-id routed)      │    │    │
│  │  └───────────┘ └─────────────────────────────────┘    │    │
│  └────────────────────────────────────────────────────────┘    │
│                                                                  │
│  ┌────────────────────────────────────────────────────────┐    │
│  │ IC_Compositor (wayland-2) 1024x600                      │    │
│  │ WaylandCompositor (Nested, Kiosk Mode, Index-based)    │    │
│  ├────────────────────────────────────────────────────────┤    │
│  │  ┌──────┐ ┌───────────┐ ┌──────┐                      │    │
│  │  │Gear  │ │Speedometer│ │Battery│                      │    │
│  │  │280px │ │  400px    │ │280px │ (Fixed layout)       │    │
│  │  └──────┘ └───────────┘ └──────┘                      │    │
│  └────────────────────────────────────────────────────────┘    │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘

vsomeip Routing Manager (UDP multicast 224.244.224.245:30490)
  ├─> HU Apps (GearApp, AmbientApp, MediaApp, HomeScreen)
  └─> IC Apps (GearState, Speedometer, BatteryMeter)
```

**핵심 특징**:
- ✅ **Nested Wayland Compositor 구조** (산업용 Kiosk Mode)
- ✅ **HU/IC 독립 Compositor** → 각각 wayland-1, wayland-2 소켓 생성
- ✅ **WaylandCompositor 최상위 구조** → 드래그 불가, 고정 위치
- ✅ **Software Rendering** → `QT_QUICK_BACKEND=software` (5-7초 OpenGL 딜레이 회피)
- ✅ **vsomeip 외부 통신** → 멀티캐스트 라우팅 (enP8p1s0)

### 1.2 프로세스 구조

```bash
# Root 프로세스
├─ weston (wayland-0, /tmp/xdg, 700 permissions)
│
├─ routingmanagerd (vsomeip 라우팅 매니저)
│
├─ HU_MainApp_Compositor (wayland-1)
│   ├─ GearApp
│   ├─ AmbientApp
│   ├─ MediaApp
│   └─ HomeScreenApp
│
└─ IC_Compositor (wayland-2)
    ├─ GearState_app (Index 0)
    ├─ Speedometer_app (Index 1)
    └─ BatteryMeter_app (Index 2)
```

**중요**: 모든 프로세스는 `sudo -E`로 실행 (XDG_RUNTIME_DIR=/tmp/xdg 접근)

---

## 2. 검증된 구성 요약

### 2.1 소프트웨어 버전

| 컴포넌트 | 버전 | 패키지 |
|---------|------|--------|
| Weston | 13.0.0 | nvidia-l4t-weston (R36.4.4) |
| Qt | 5.15.3 | system Qt (aarch64-linux-gnu) |
| vsomeip | 3.5.8 | 소스 빌드 |
| CommonAPI | 3.2.4 | 소스 빌드 |
| Kernel | 5.15.x | NVIDIA L4T R36.4.4 |

### 2.2 HU 앱 (4개)

| 앱 | 해상도 | 소켓 | 설명 |
|----|--------|------|------|
| **GearApp** | 130x1000 | wayland-1 | 왼쪽 패널 (기어 상태) |
| **AmbientApp** | 1790x1000 | wayland-1 | 메인 영역 (앰비언트) |
| **MediaApp** | 1790x1000 | wayland-1 | 메인 영역 (미디어) |
| **HomeScreenApp** | 1790x1000 | wayland-1 | 메인 영역 (홈) |

**Routing**: HU_MainApp_Compositor가 app-id + title 기반 라우팅

### 2.3 IC 앱 (3개)

| 앱 | 해상도 | 소켓 | Index |
|----|--------|------|-------|
| **GearState_app** | 280x600 | wayland-2 | 0 (왼쪽) |
| **Speedometer_app** | 400x600 | wayland-2 | 1 (중앙) |
| **BatteryMeter_app** | 280x600 | wayland-2 | 2 (오른쪽) |

**Routing**: IC_Compositor가 시작 순서 기반 Index 라우팅

### 2.4 빌드 스크립트 구조

```
DES_Head-Unit/
├── app/
│   ├── HU_MainApp/
│   │   ├── build_compositor.sh          # HU Compositor 빌드
│   │   └── src/main_compositor.cpp      # WaylandCompositor 최상위
│   ├── GearApp/
│   ├── AmbientApp/
│   ├── MediaApp/
│   ├── HomeScreenApp/
│   ├── IC_Compositor/
│   │   ├── build.sh                      # IC Compositor 빌드
│   │   ├── src/main.cpp                  # WaylandCompositor 최상위
│   │   └── qml/main.qml                  # Index-based routing
│   ├── GearState_app/
│   ├── Speedometer_app/
│   ├── BatteryMeter_app/
│   └── config/
│       ├── build_all_ic_apps.sh         # IC 4개 일괄 빌드
│       ├── run_ic_system.sh             # IC 시스템 실행
│       └── start_routing_manager.sh     # vsomeip 라우팅 매니저
├── run-jetson-wayland-full.sh           # HU 시스템 실행
└── install_folder/                       # vsomeip, CommonAPI 라이브러리
```

### 2.5 환경 변수 설정

#### 모든 앱 공통
```bash
export XDG_RUNTIME_DIR=/tmp/xdg
export QT_QPA_PLATFORM=wayland
export QSG_RENDER_LOOP=basic
export QT_QUICK_BACKEND=software          # Software rendering (필수)
export LD_LIBRARY_PATH=/path/to/install_folder/lib:$LD_LIBRARY_PATH
```

#### HU Compositor
```bash
export WAYLAND_DISPLAY=wayland-0          # Weston 연결
export QT_WAYLAND_DISABLE_WINDOWDECORATION=1
```

#### HU Apps
```bash
export WAYLAND_DISPLAY=wayland-1          # HU Compositor 연결
export VSOMEIP_CONFIGURATION=/path/to/vsomeip_ecu2.json
```

#### IC Compositor
```bash
export WAYLAND_DISPLAY=wayland-0          # Weston 연결
export QT_WAYLAND_DISABLE_WINDOWDECORATION=1
```

#### IC Apps
```bash
export WAYLAND_DISPLAY=wayland-2          # IC Compositor 연결
export VSOMEIP_CONFIGURATION=/path/to/vsomeip_${app}.json
export COMMONAPI_CONFIG=/path/to/commonapi_${app}.ini
```

---

## 3. Yocto Layer 구조

### 3.1 전체 Layer 구성

```
poky/                                     # Yocto base
├── meta/
├── meta-poky/
└── meta-yocto-bsp/

meta-tegra/                               # NVIDIA Jetson BSP
├── recipes-bsp/                          # L4T drivers, firmware
├── recipes-graphics/                     # Weston 13.0.0, DRM
└── conf/machine/jetson-orin-nano.conf

meta-openembedded/
└── meta-oe/                              # vsomeip dependencies (boost)

meta-qt5/                                 # Qt 5.15.x
├── recipes-qt/
└── classes/qmake5.bbclass

meta-jetson-headunit/                     # ★ 신규 레이어
├── conf/
│   ├── layer.conf
│   └── machine/
│       └── jetson-orin-nano-headunit.conf
├── recipes-core/
│   └── images/
│       └── jetson-headunit-image.bb     # 최종 이미지
├── recipes-apps/
│   ├── hu-compositor/
│   ├── ic-compositor/
│   ├── gearapp/
│   ├── ambientapp/
│   ├── mediaapp/
│   ├── homescreenapp/
│   ├── gearstate-app/
│   ├── speedometer-app/
│   └── batterymeter-app/
├── recipes-middleware/
│   ├── vsomeip/
│   └── commonapi/
├── recipes-graphics/
│   └── weston/
│       └── weston_%.bbappend
└── recipes-support/
    └── hu-ic-scripts/                   # 시작 스크립트
```

### 3.2 meta-jetson-headunit/conf/layer.conf

```python
# Layer configuration for meta-jetson-headunit
BBPATH =. "${LAYERDIR}:"

BBFILES += "${LAYERDIR}/recipes-*/*/*.bb \
            ${LAYERDIR}/recipes-*/*/*.bbappend"

BBFILE_COLLECTIONS += "jetson-headunit"
BBFILE_PATTERN_jetson-headunit = "^${LAYERDIR}/"
BBFILE_PRIORITY_jetson-headunit = "10"

LAYERDEPENDS_jetson-headunit = "core tegra openembedded-layer qt5-layer"
LAYERSERIES_COMPAT_jetson-headunit = "kirkstone"
```

### 3.3 Machine 설정

**meta-jetson-headunit/conf/machine/jetson-orin-nano-headunit.conf**:

```bash
# Jetson Orin Nano HU/IC System Machine Configuration
require conf/machine/jetson-orin-nano.conf

MACHINE = "jetson-orin-nano-headunit"
MACHINEOVERRIDES =. "jetson-orin-nano:"

# Display configuration
PREFERRED_PROVIDER_virtual/egl = "nvidia-egl"
PREFERRED_PROVIDER_virtual/libgles2 = "nvidia-egl"
DISTRO_FEATURES:append = " wayland opengl"
DISTRO_FEATURES:remove = "x11"

# Weston configuration
PACKAGECONFIG:append:pn-weston = " drm systemd"
PACKAGECONFIG:remove:pn-weston = "x11 fbdev"

# Qt configuration
PACKAGECONFIG:append:pn-qtbase = " eglfs wayland"
PACKAGECONFIG:append:pn-qtwayland = " wayland-compositor"

# systemd init
INIT_MANAGER = "systemd"

# Image features
IMAGE_FEATURES += "ssh-server-openssh"
EXTRA_IMAGE_FEATURES += "debug-tweaks tools-debug"

# Kernel modules
KERNEL_MODULE_AUTOLOAD += "nvidia_drm"
KERNEL_MODULE_PROBECONF += "nvidia_drm"
module_conf_nvidia_drm = "options nvidia_drm modeset=1"
```

---

## 4. Recipe 작성 가이드

### 4.1 HU_MainApp_Compositor Recipe

**recipes-apps/hu-compositor/hu-compositor_2.0.bb**:

```python
SUMMARY = "HU Main App Wayland Compositor"
DESCRIPTION = "Nested Wayland Compositor for Head Unit applications (Kiosk Mode)"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "qtbase qtdeclarative qtwayland vsomeip commonapi"
RDEPENDS:${PN} = "qtbase qtdeclarative qtwayland-qmlplugins weston"

SRC_URI = "git://github.com/your-org/DES_Head-Unit.git;protocol=https;branch=JetPack"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git/app/HU_MainApp"

inherit qmake5 systemd

# Qt Wayland Compositor requires these
EXTRA_QMAKEVARS_PRE += "CONFIG+=qtquickcompiler"

do_configure:prepend() {
    export PKG_CONFIG_PATH="${STAGING_LIBDIR}/pkgconfig"
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/build_compositor/HU_MainApp_Compositor ${D}${bindir}/

    install -d ${D}${datadir}/hu-compositor
    cp -r ${S}/qml ${D}${datadir}/hu-compositor/
    cp -r ${S}/asset ${D}${datadir}/hu-compositor/

    # systemd service
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/hu-compositor.service ${D}${systemd_system_unitdir}/
}

SYSTEMD_SERVICE:${PN} = "hu-compositor.service"

FILES:${PN} += "${bindir}/HU_MainApp_Compositor \
                ${datadir}/hu-compositor/* \
                ${systemd_system_unitdir}/hu-compositor.service"
```

**hu-compositor.service**:

```ini
[Unit]
Description=HU Main App Wayland Compositor
After=weston.service vsomeip-routing.service
Requires=weston.service vsomeip-routing.service
PartOf=hu-system.target

[Service]
Type=simple
User=root
Environment="XDG_RUNTIME_DIR=/tmp/xdg"
Environment="WAYLAND_DISPLAY=wayland-0"
Environment="QT_QPA_PLATFORM=wayland"
Environment="QT_WAYLAND_DISABLE_WINDOWDECORATION=1"
Environment="QSG_RENDER_LOOP=basic"
Environment="QT_QUICK_BACKEND=software"
Environment="LD_LIBRARY_PATH=/usr/lib:/usr/local/lib"
ExecStart=/usr/bin/HU_MainApp_Compositor
Restart=always
RestartSec=3

[Install]
WantedBy=hu-system.target
```

### 4.2 IC_Compositor Recipe

**recipes-apps/ic-compositor/ic-compositor_1.0.bb**:

```python
SUMMARY = "IC Wayland Compositor"
DESCRIPTION = "Nested Wayland Compositor for Instrument Cluster (Kiosk Mode, Index-based routing)"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "qtbase qtdeclarative qtwayland"
RDEPENDS:${PN} = "qtbase qtdeclarative qtwayland-qmlplugins weston"

SRC_URI = "git://github.com/your-org/DES_Head-Unit.git;protocol=https;branch=JetPack"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git/app/IC_Compositor"

inherit qmake5 cmake systemd

do_configure() {
    cmake -B ${B} -S ${S} \
        -DCMAKE_INSTALL_PREFIX=${D}${prefix} \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake
}

do_compile() {
    cmake --build ${B} -- -j${@oe.utils.cpu_count()}
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/IC_Compositor ${D}${bindir}/

    # systemd service
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/ic-compositor.service ${D}${systemd_system_unitdir}/
}

SYSTEMD_SERVICE:${PN} = "ic-compositor.service"

FILES:${PN} += "${bindir}/IC_Compositor \
                ${systemd_system_unitdir}/ic-compositor.service"
```

**ic-compositor.service**:

```ini
[Unit]
Description=IC Wayland Compositor
After=weston.service vsomeip-routing.service
Requires=weston.service vsomeip-routing.service
PartOf=ic-system.target

[Service]
Type=simple
User=root
Environment="XDG_RUNTIME_DIR=/tmp/xdg"
Environment="WAYLAND_DISPLAY=wayland-0"
Environment="QT_QPA_PLATFORM=wayland"
Environment="QT_WAYLAND_DISABLE_WINDOWDECORATION=1"
Environment="QSG_RENDER_LOOP=basic"
Environment="QT_QUICK_BACKEND=software"
ExecStart=/usr/bin/IC_Compositor
Restart=always
RestartSec=3

[Install]
WantedBy=ic-system.target
```

### 4.3 HU App Recipe 예시 (GearApp)

**recipes-apps/gearapp/gearapp_1.0.bb**:

```python
SUMMARY = "Gear Selection App"
DESCRIPTION = "HU Gear App - wayland-1 client"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "qtbase qtdeclarative vsomeip commonapi"
RDEPENDS:${PN} = "qtbase qtdeclarative qtquickcontrols2 hu-compositor"

SRC_URI = "git://github.com/your-org/DES_Head-Unit.git;protocol=https;branch=JetPack"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git/app/GearApp"

inherit qmake5 systemd

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/build/GearApp ${D}${bindir}/

    install -d ${D}${datadir}/gearapp
    cp -r ${S}/qml ${D}${datadir}/gearapp/
    cp -r ${S}/config ${D}${datadir}/gearapp/

    # systemd service
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/gearapp.service ${D}${systemd_system_unitdir}/
}

SYSTEMD_SERVICE:${PN} = "gearapp.service"

FILES:${PN} += "${bindir}/GearApp \
                ${datadir}/gearapp/* \
                ${systemd_system_unitdir}/gearapp.service"
```

**gearapp.service**:

```ini
[Unit]
Description=Gear Selection App
After=hu-compositor.service
Requires=hu-compositor.service
PartOf=hu-system.target

[Service]
Type=simple
User=root
Environment="XDG_RUNTIME_DIR=/tmp/xdg"
Environment="WAYLAND_DISPLAY=wayland-1"
Environment="QT_QPA_PLATFORM=wayland"
Environment="QSG_RENDER_LOOP=basic"
Environment="QT_QUICK_BACKEND=software"
Environment="VSOMEIP_CONFIGURATION=/usr/share/gearapp/config/vsomeip_ecu2.json"
Environment="LD_LIBRARY_PATH=/usr/lib:/usr/local/lib"
ExecStart=/usr/bin/GearApp
Restart=always
RestartSec=3

[Install]
WantedBy=hu-system.target
```

### 4.4 IC App Recipe 예시 (GearState_app)

**recipes-apps/gearstate-app/gearstate-app_1.0.bb**:

```python
SUMMARY = "GearState IC App"
DESCRIPTION = "IC GearState App - wayland-2 client (Index 0)"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "qtbase qtdeclarative vsomeip commonapi"
RDEPENDS:${PN} = "qtbase qtdeclarative ic-compositor"

SRC_URI = "git://github.com/your-org/DES_Head-Unit.git;protocol=https;branch=JetPack"
SRCREV = "${AUTOREV}"

S = "${WORKDIR}/git/app/GearState_app"

inherit cmake systemd

do_configure() {
    cmake -B ${B} -S ${S} \
        -DCMAKE_INSTALL_PREFIX=${D}${prefix} \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_PREFIX_PATH=/usr/lib/aarch64-linux-gnu/cmake
}

do_compile() {
    cmake --build ${B} -- -j${@oe.utils.cpu_count()}
}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/GearState_app ${D}${bindir}/

    install -d ${D}${datadir}/gearstate-app
    cp -r ${S}/qml ${D}${datadir}/gearstate-app/
    cp -r ${S}/config ${D}${datadir}/gearstate-app/

    # systemd service
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/gearstate-app.service ${D}${systemd_system_unitdir}/
}

SYSTEMD_SERVICE:${PN} = "gearstate-app.service"

FILES:${PN} += "${bindir}/GearState_app \
                ${datadir}/gearstate-app/* \
                ${systemd_system_unitdir}/gearstate-app.service"
```

**gearstate-app.service**:

```ini
[Unit]
Description=GearState IC App (Index 0)
After=ic-compositor.service
Requires=ic-compositor.service
PartOf=ic-system.target

[Service]
Type=simple
User=root
Environment="XDG_RUNTIME_DIR=/tmp/xdg"
Environment="WAYLAND_DISPLAY=wayland-2"
Environment="QT_QPA_PLATFORM=wayland"
Environment="QSG_RENDER_LOOP=basic"
Environment="QT_QUICK_BACKEND=software"
Environment="VSOMEIP_CONFIGURATION=/usr/share/gearstate-app/config/vsomeip_gearstate_app.json"
Environment="COMMONAPI_CONFIG=/usr/share/gearstate-app/config/commonapi_gearstate_app.ini"
Environment="LD_LIBRARY_PATH=/usr/lib:/usr/local/lib"
ExecStartPre=/bin/sleep 2
ExecStart=/usr/bin/GearState_app
Restart=always
RestartSec=3

[Install]
WantedBy=ic-system.target
```

**중요**: IC 앱은 시작 순서가 중요 (Index-based routing)
- GearState (Index 0) → Speedometer (Index 1) → BatteryMeter (Index 2)
- `ExecStartPre=/bin/sleep 2` 등으로 순차 시작 보장

### 4.5 vsomeip Routing Manager Recipe

**recipes-middleware/vsomeip/vsomeip_3.5.8.bb**:

```python
SUMMARY = "vsomeip - SOME/IP implementation"
DESCRIPTION = "vsomeip is a SOME/IP implementation for embedded Linux"
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=815ca599c9df247a0c7f619bab123dad"

DEPENDS = "boost"

SRC_URI = "git://github.com/COVESA/vsomeip.git;protocol=https;branch=master"
SRCREV = "3.5.8"

S = "${WORKDIR}/git"

inherit cmake systemd

EXTRA_OECMAKE = "-DENABLE_SIGNAL_HANDLING=1 \
                 -DDIAGNOSIS_ADDRESS=0x10 \
                 -DCMAKE_BUILD_TYPE=Release"

do_install:append() {
    # systemd service for routing manager
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/vsomeip-routing.service ${D}${systemd_system_unitdir}/

    # Configuration
    install -d ${D}${sysconfdir}/vsomeip
    install -m 0644 ${WORKDIR}/routing_manager_ecu2.json ${D}${sysconfdir}/vsomeip/
}

SYSTEMD_SERVICE:${PN} = "vsomeip-routing.service"

FILES:${PN} += "${libdir}/*.so.* \
                ${bindir}/routingmanagerd \
                ${sysconfdir}/vsomeip/* \
                ${systemd_system_unitdir}/vsomeip-routing.service"
```

**vsomeip-routing.service**:

```ini
[Unit]
Description=vsomeip Routing Manager
After=network.target
Before=hu-compositor.service ic-compositor.service

[Service]
Type=simple
User=root
Environment="LD_LIBRARY_PATH=/usr/lib:/usr/local/lib"
Environment="VSOMEIP_CONFIGURATION=/etc/vsomeip/routing_manager_ecu2.json"
ExecStartPre=/bin/rm -rf /tmp/vsomeip-*
ExecStart=/usr/bin/routingmanagerd
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

---

## 5. Weston 설정

### 5.1 Weston Recipe Append

**recipes-graphics/weston/weston_%.bbappend**:

```python
FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://weston-jetson.ini \
            file://weston.service"

do_install:append() {
    # Default configuration
    install -d ${D}${sysconfdir}/xdg/weston
    install -m 0644 ${WORKDIR}/weston-jetson.ini ${D}${sysconfdir}/xdg/weston/weston.ini

    # systemd service override
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/weston.service ${D}${systemd_system_unitdir}/
}

# NVIDIA DRM backend
PACKAGECONFIG = "drm systemd"
```

### 5.2 Weston Configuration

**files/weston-jetson.ini**:

```ini
[core]
backend=drm-backend.so
require-input=false

[output]
name=DP-1
mode=1920x1080
transform=normal

[shell]
panel-position=none
locking=false
background-color=0xff000000

[libinput]
enable-tap=true
```

### 5.3 Weston systemd Service

**files/weston.service**:

```ini
[Unit]
Description=Weston Wayland Compositor
After=multi-user.target

[Service]
Type=simple
User=root
Environment="XDG_RUNTIME_DIR=/tmp/xdg"
ExecStartPre=/bin/sh -c 'systemctl stop gdm; pkill -9 Xorg; modprobe nvidia_drm modeset=1'
ExecStartPre=/bin/mkdir -p /tmp/xdg
ExecStartPre=/bin/chmod 700 /tmp/xdg
ExecStartPre=/bin/rm -rf /tmp/xdg/wayland-*
ExecStart=/usr/bin/weston --idle-time=0 --log=/var/log/weston.log
Restart=always
RestartSec=5

[Install]
WantedBy=graphical.target
```

---

## 6. systemd 서비스 구성

### 6.1 Target 구성

**recipes-support/hu-ic-scripts/files/hu-system.target**:

```ini
[Unit]
Description=Head Unit System (HU Apps)
Requires=weston.service vsomeip-routing.service
After=weston.service vsomeip-routing.service

[Install]
WantedBy=graphical.target
```

**recipes-support/hu-ic-scripts/files/ic-system.target**:

```ini
[Unit]
Description=Instrument Cluster System (IC Apps)
Requires=weston.service vsomeip-routing.service
After=weston.service vsomeip-routing.service

[Install]
WantedBy=graphical.target
```

### 6.2 서비스 시작 순서

```
graphical.target
  ├─> weston.service (wayland-0)
  │     └─> vsomeip-routing.service
  │           ├─> hu-system.target
  │           │     ├─> hu-compositor.service (wayland-1)
  │           │     │     ├─> gearapp.service
  │           │     │     ├─> ambientapp.service
  │           │     │     ├─> mediaapp.service
  │           │     │     └─> homescreenapp.service
  │           │     │
  │           │     └─> ic-system.target
  │           │           ├─> ic-compositor.service (wayland-2)
  │           │           │     ├─> gearstate-app.service (Index 0)
  │           │           │     ├─> speedometer-app.service (Index 1)
  │           │           │     └─> batterymeter-app.service (Index 2)
```

### 6.3 Multicast Routing 설정

**recipes-support/hu-ic-scripts/files/setup-multicast.service**:

```ini
[Unit]
Description=Setup Multicast Routing for vsomeip
After=network.target
Before=vsomeip-routing.service

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=/bin/sh -c 'ip route add 224.0.0.0/4 dev enP8p1s0 2>/dev/null || true'

[Install]
WantedBy=multi-user.target
```

---

## 7. 빌드 실행

### 7.1 빌드 환경 설정

```bash
# 1. Yocto 다운로드 (Kirkstone LTS)
git clone -b kirkstone git://git.yoctoproject.org/poky.git
cd poky

# 2. 필수 레이어 다운로드
git clone -b kirkstone https://github.com/OE4T/meta-tegra.git
git clone -b kirkstone https://github.com/openembedded/meta-openembedded.git
git clone -b kirkstone https://github.com/meta-qt5/meta-qt5.git

# 3. 커스텀 레이어 추가
git clone https://github.com/your-org/meta-jetson-headunit.git

# 4. 빌드 환경 초기화
source oe-init-build-env build-jetson-headunit
```

### 7.2 local.conf 설정

**build-jetson-headunit/conf/local.conf**:

```bash
# Machine selection
MACHINE = "jetson-orin-nano-headunit"

# Parallel build
BB_NUMBER_THREADS = "8"
PARALLEL_MAKE = "-j 8"

# Download directory
DL_DIR ?= "${TOPDIR}/../downloads"
SSTATE_DIR ?= "${TOPDIR}/../sstate-cache"

# systemd
DISTRO_FEATURES:append = " systemd wayland opengl"
DISTRO_FEATURES:remove = "x11 sysvinit"
VIRTUAL-RUNTIME_init_manager = "systemd"
VIRTUAL-RUNTIME_initscripts = "systemd-compat-units"

# Package management
PACKAGE_CLASSES = "package_ipk"
EXTRA_IMAGE_FEATURES = "debug-tweaks ssh-server-openssh tools-debug"

# License
LICENSE_FLAGS_ACCEPTED = "commercial"

# Image size
IMAGE_ROOTFS_EXTRA_SPACE = "1048576"  # 1GB extra

# Network (for vsomeip)
PREFERRED_PROVIDER_virtual/kernel = "linux-tegra"
IMAGE_INSTALL:append = " dhcp-client iptables"
```

### 7.3 bblayers.conf 설정

**build-jetson-headunit/conf/bblayers.conf**:

```python
BBLAYERS ?= " \
  /path/to/poky/meta \
  /path/to/poky/meta-poky \
  /path/to/poky/meta-yocto-bsp \
  /path/to/meta-tegra \
  /path/to/meta-openembedded/meta-oe \
  /path/to/meta-openembedded/meta-networking \
  /path/to/meta-qt5 \
  /path/to/meta-jetson-headunit \
"
```

### 7.4 이미지 Recipe

**recipes-core/images/jetson-headunit-image.bb**:

```python
SUMMARY = "Jetson Head Unit / Instrument Cluster Image"
LICENSE = "MIT"

inherit core-image

# Base system
IMAGE_INSTALL = "\
    packagegroup-core-boot \
    ${CORE_IMAGE_EXTRA_INSTALL} \
"

# System utilities
IMAGE_INSTALL += "\
    systemd \
    systemd-analyze \
    util-linux \
    bash \
    procps \
    iproute2 \
    iptables \
    dhcp-client \
    openssh \
"

# Graphics stack
IMAGE_INSTALL += "\
    weston \
    weston-init \
    weston-examples \
    mesa \
    libdrm \
    nvidia-driver \
"

# Qt5
IMAGE_INSTALL += "\
    qtbase \
    qtbase-plugins \
    qtdeclarative \
    qtdeclarative-qmlplugins \
    qtquickcontrols2 \
    qtquickcontrols2-qmlplugins \
    qtwayland \
    qtwayland-qmlplugins \
    qtwayland-plugins \
"

# Middleware
IMAGE_INSTALL += "\
    vsomeip \
    commonapi \
    boost \
"

# HU Applications
IMAGE_INSTALL += "\
    hu-compositor \
    gearapp \
    ambientapp \
    mediaapp \
    homescreenapp \
"

# IC Applications
IMAGE_INSTALL += "\
    ic-compositor \
    gearstate-app \
    speedometer-app \
    batterymeter-app \
"

# Scripts and configs
IMAGE_INSTALL += "\
    hu-ic-scripts \
"

# Image features
IMAGE_FEATURES += "splash package-management ssh-server-openssh"

# Rootfs size
IMAGE_ROOTFS_SIZE = "4096000"  # 4GB
IMAGE_ROOTFS_EXTRA_SPACE = "1048576"  # +1GB
```

### 7.5 빌드 실행

```bash
# 전체 이미지 빌드
bitbake jetson-headunit-image

# 개별 앱만 빌드
bitbake hu-compositor
bitbake ic-compositor
bitbake gearapp

# 클린 빌드
bitbake -c cleanall jetson-headunit-image
bitbake jetson-headunit-image
```

### 7.6 빌드 결과물

```
build-jetson-headunit/tmp/deploy/images/jetson-orin-nano-headunit/
├── jetson-headunit-image-jetson-orin-nano-headunit.tar.gz
├── jetson-headunit-image-jetson-orin-nano-headunit.rootfs.ext4
├── jetson-headunit-image-jetson-orin-nano-headunit.wic.gz
└── modules-*.tgz
```

---

## 8. 배포 및 검증

### 8.1 이미지 플래싱

```bash
# 1. SD 카드/eMMC 플래싱 (NVIDIA SDK Manager 사용)
# 또는 직접 wic 이미지 플래싱:

sudo dd if=jetson-headunit-image-jetson-orin-nano-headunit.wic.gz \
        of=/dev/sdX bs=4M status=progress
sync
```

### 8.2 첫 부팅 확인

```bash
# SSH 접속
ssh root@<jetson-ip>

# 서비스 상태 확인
systemctl status weston.service
systemctl status vsomeip-routing.service
systemctl status hu-compositor.service
systemctl status ic-compositor.service

# 프로세스 확인
ps aux | grep -E "weston|routingmanagerd|HU_MainApp|IC_Compositor"

# 소켓 확인
ls -la /tmp/xdg/wayland-*
# Expected:
# wayland-0 (Weston)
# wayland-1 (HU Compositor)
# wayland-2 (IC Compositor)

# 로그 확인
journalctl -u weston.service
journalctl -u hu-compositor.service
journalctl -u ic-compositor.service
journalctl -u gearapp.service
```

### 8.3 검증 체크리스트

#### ✅ Weston 동작
```bash
# Weston 프로세스 존재
ps aux | grep weston

# wayland-0 소켓 생성
ls /tmp/xdg/wayland-0

# Weston 로그 정상
journalctl -u weston.service | grep "Output 'DP-1' enabled"
```

#### ✅ vsomeip Routing Manager
```bash
# 프로세스 존재
ps aux | grep routingmanagerd

# 소켓 생성
ls /tmp/vsomeip-0

# 멀티캐스트 라우팅 설정
ip route | grep "224.0.0.0/4"
```

#### ✅ HU System
```bash
# HU Compositor 동작
ps aux | grep HU_MainApp_Compositor

# wayland-1 소켓 생성
ls /tmp/xdg/wayland-1

# HU 앱 4개 동작
ps aux | grep -E "GearApp|AmbientApp|MediaApp|HomeScreen"
```

#### ✅ IC System
```bash
# IC Compositor 동작
ps aux | grep IC_Compositor

# wayland-2 소켓 생성
ls /tmp/xdg/wayland-2

# IC 앱 3개 동작 (순서 확인)
ps aux | grep -E "GearState|Speedometer|BatteryMeter"
```

### 8.4 성능 측정

```bash
# 부팅 시간 분석
systemd-analyze
systemd-analyze blame
systemd-analyze critical-chain weston.service

# 메모리 사용량
free -h
ps aux --sort=-rss | head -20

# CPU 사용률
top -b -n 1 | head -20

# GPU 상태 (NVIDIA)
nvidia-smi
```

---

## 9. 듀얼 디스플레이 전환 (향후)

### 9.1 현재 제약사항

**하드웨어**: Jetson Orin Nano는 단일 DisplayPort만 제공
- **옵션 1**: DP MST Hub 사용 (DisplayPort Multi-Stream Transport)
- **옵션 2**: USB-C DP Alt Mode 추가 (지원 여부 확인 필요)

### 9.2 DP MST Hub 구성 (권장)

#### 하드웨어
```
Jetson Orin Nano DP-1
  └─> DP MST Hub (예: StarTech MSTDP123DP)
        ├─> Display 1 (HU) @ 1024x600
        └─> Display 2 (IC) @ 1024x600
```

#### Weston 설정 변경

**weston-dual-display.ini**:

```ini
[core]
backend=drm-backend.so

# HU Display (MST Stream 1)
[output]
name=DP-1-1
mode=1024x600
transform=normal

# IC Display (MST Stream 2)
[output]
name=DP-1-2
mode=1024x600
transform=normal

[shell]
panel-position=none
locking=false
```

#### Compositor 할당 변경

**HU Compositor**: DP-1-1에 fullscreen
**IC Compositor**: DP-1-2에 fullscreen

Weston이 각 output에 compositor를 fullscreen으로 배치하도록 설정 필요.

### 9.3 Yocto Recipe 변경사항

#### Weston 설정 추가

```python
# recipes-graphics/weston/weston_%.bbappend
SRC_URI += "file://weston-dual-display.ini"

do_install:append() {
    install -m 0644 ${WORKDIR}/weston-dual-display.ini \
        ${D}${sysconfdir}/xdg/weston/weston-dual.ini
}
```

#### systemd 서비스 수정

```ini
# Weston 실행 시 dual display config 사용
ExecStart=/usr/bin/weston --config=/etc/xdg/weston/weston-dual.ini \
          --idle-time=0 --log=/var/log/weston.log
```

#### HU/IC Compositor 환경 변수

**HU Compositor**:
```bash
Environment="WAYLAND_DISPLAY=wayland-0"
Environment="WESTON_OUTPUT_NAME=DP-1-1"  # HU Display
```

**IC Compositor**:
```bash
Environment="WAYLAND_DISPLAY=wayland-0"
Environment="WESTON_OUTPUT_NAME=DP-1-2"  # IC Display
```

---

## 10. 문제 해결 가이드

### 10.1 Weston 시작 실패

**증상**: Weston 프로세스가 시작되지 않음

**확인**:
```bash
journalctl -u weston.service
cat /var/log/weston.log
```

**가능 원인**:
1. **nvidia_drm 미로드**
   ```bash
   lsmod | grep nvidia_drm
   # 없으면:
   modprobe nvidia_drm modeset=1
   ```

2. **DP-1 연결 안됨**
   ```bash
   ls /sys/class/drm/card0/card0-DP-1/status
   cat /sys/class/drm/card0/card0-DP-1/status  # should be "connected"
   ```

3. **XDG_RUNTIME_DIR 권한**
   ```bash
   ls -ld /tmp/xdg
   # drwx------ root root 필요
   chmod 700 /tmp/xdg
   ```

### 10.2 Compositor 시작 실패

**증상**: wayland-1 또는 wayland-2 소켓 생성 안됨

**확인**:
```bash
journalctl -u hu-compositor.service
journalctl -u ic-compositor.service
```

**가능 원인**:
1. **Weston 미실행**
   ```bash
   systemctl status weston.service
   ls /tmp/xdg/wayland-0
   ```

2. **Qt Wayland 플러그인 없음**
   ```bash
   ls /usr/lib/qt5/plugins/wayland-graphics-integration-server/
   # libqt-wayland-compositor-*.so 파일들 있어야 함
   ```

3. **QML 파일 못 찾음**
   ```bash
   # Recipe에서 qml 파일 제대로 설치했는지 확인
   ls /usr/share/hu-compositor/qml/
   ```

### 10.3 앱 시작 실패

**증상**: HU/IC 앱이 시작되지 않음

**확인**:
```bash
journalctl -u gearapp.service
journalctl -u gearstate-app.service
```

**가능 원인**:
1. **Compositor 소켓 없음**
   ```bash
   ls /tmp/xdg/wayland-1  # HU apps
   ls /tmp/xdg/wayland-2  # IC apps
   ```

2. **vsomeip 설정 파일 없음**
   ```bash
   ls /usr/share/gearapp/config/vsomeip_ecu2.json
   ```

3. **LD_LIBRARY_PATH 오류**
   ```bash
   ldd /usr/bin/GearApp
   # vsomeip, commonapi 라이브러리 찾을 수 있는지 확인
   ```

### 10.4 IC 앱 레이아웃 깨짐

**증상**: IC 앱이 잘못된 위치에 표시

**원인**: Index-based routing이므로 시작 순서 중요

**해결**:
```bash
# IC 앱 서비스 순차 시작 확인
systemctl list-units ic-system.target
systemctl list-dependencies ic-system.target

# GearState (Index 0) → Speedometer (Index 1) → BatteryMeter (Index 2)
# ExecStartPre에 sleep 추가로 순서 보장
```

### 10.5 vsomeip 통신 실패

**증상**: 앱 간 vsomeip 메시지 송수신 안됨

**확인**:
```bash
# Routing manager 동작 확인
ps aux | grep routingmanagerd

# 멀티캐스트 라우팅 확인
ip route | grep 224.0.0.0

# 네트워크 인터페이스 확인
ip addr show enP8p1s0
```

**해결**:
```bash
# 멀티캐스트 라우팅 수동 추가
ip route add 224.0.0.0/4 dev enP8p1s0

# systemd service로 자동화
systemctl enable setup-multicast.service
```

### 10.6 성능 저하 (느린 렌더링)

**증상**: GUI 반응이 느리거나 5-7초 지연

**원인**: OpenGL 하드웨어 렌더링 사용 시 nested compositor 동기화 이슈

**해결**: Software Rendering 강제
```bash
# 모든 앱 환경 변수 확인
systemctl cat gearapp.service | grep QT_QUICK_BACKEND
# QT_QUICK_BACKEND=software 있어야 함

# 없으면 service 파일 수정
Environment="QT_QUICK_BACKEND=software"
Environment="QSG_RENDER_LOOP=basic"
```

---

## 11. 참고 자료

### 11.1 NVIDIA 공식 문서
- **Weston/Wayland**: https://docs.nvidia.com/jetson/archives/r36.4.4/DeveloperGuide/SD/WindowingSystems/WestonWayland.html
- **L4T Documentation**: https://docs.nvidia.com/jetson/l4t/index.html
- **Jetson Linux Driver Package**: JetPack 6.1 (R36.4.4)

### 11.2 Yocto 문서
- **Yocto Project**: https://www.yoctoproject.org/docs/
- **meta-tegra**: https://github.com/OE4T/meta-tegra
- **Kirkstone Manual**: https://docs.yoctoproject.org/kirkstone/

### 11.3 프로젝트 참조
- **기존 Jetson Wayland 가이드**: `/docs/JETSON_WESTON_WAYLAND_GUIDE.md`
- **실행 스크립트**: `run-jetson-wayland-full.sh`, `run_ic_system.sh`
- **빌드 스크립트**: `app/config/build_all_ic_apps.sh`

---

## 12. 요약 체크리스트

### ✅ Recipe 작성 완료
- [ ] hu-compositor_2.0.bb
- [ ] ic-compositor_1.0.bb
- [ ] gearapp_1.0.bb, ambientapp_1.0.bb, mediaapp_1.0.bb, homescreenapp_1.0.bb
- [ ] gearstate-app_1.0.bb, speedometer-app_1.0.bb, batterymeter-app_1.0.bb
- [ ] vsomeip_3.5.8.bb, commonapi_3.2.4.bb
- [ ] weston_%.bbappend

### ✅ systemd 서비스 작성 완료
- [ ] weston.service
- [ ] vsomeip-routing.service
- [ ] hu-compositor.service, ic-compositor.service
- [ ] hu-system.target, ic-system.target
- [ ] 각 앱별 service 파일 (gearapp.service 등)
- [ ] setup-multicast.service

### ✅ 설정 파일 작성 완료
- [ ] weston-jetson.ini (단일 디스플레이)
- [ ] weston-dual-display.ini (듀얼 디스플레이, 향후)
- [ ] vsomeip config JSON (routing_manager_ecu2.json 등)
- [ ] commonapi config INI

### ✅ Yocto 환경 설정 완료
- [ ] local.conf (MACHINE, DISTRO_FEATURES)
- [ ] bblayers.conf (meta-tegra, meta-qt5, meta-jetson-headunit)
- [ ] jetson-orin-nano-headunit.conf
- [ ] jetson-headunit-image.bb

### ✅ 빌드 실행
```bash
source oe-init-build-env build-jetson-headunit
bitbake jetson-headunit-image
```

### ✅ 플래싱 및 검증
```bash
# 이미지 플래싱
sudo dd if=jetson-headunit-image.wic.gz of=/dev/sdX bs=4M

# 부팅 후 검증
systemctl status weston
systemctl status hu-system.target
systemctl status ic-system.target
ps aux | grep -E "HU_MainApp|IC_Compositor|GearApp"
ls /tmp/xdg/wayland-*
```

---

**작성 완료: 2026년 1월 27일**  
**검증 상태**: ✅ HU 4개 앱 + IC 3개 앱 완벽 동작 (단일 디스플레이)  
**향후 작업**: DP MST Hub 듀얼 디스플레이 전환
