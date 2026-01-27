# IC Apps Dependencies for Yocto Build
**Date:** 2026-01-20  
**Platform:** Jetson Orin Nano  
**Apps:** BatteryMeter_app, GearState_app, Speedometer_app  

---

## 1. Build Verification Status

### ✅ Successfully Built Apps
All three IC apps have been built successfully on Jetson Ubuntu 22.04:

| App | Build Status | Executable Path |
|-----|--------------|-----------------|
| BatteryMeter_app | ✅ Success | `/app/BatteryMeter_app/build/BatteryMeter_app` |
| GearState_app | ✅ Success | `/app/GearState_app/build/GearState_app` |
| Speedometer_app | ✅ Success | `/app/Speedometer_app/build/Speedometer_app` |

**Build Script:** `/app/config/build_all_ic_apps.sh`

---

## 2. Verified Dependencies

### Core Build Dependencies

#### Qt5 (Version: 5.15.3)
```bash
# System Qt5 packages used
Package: qtbase5-dev
Package: qtdeclarative5-dev
Package: qtquickcontrols2-5-dev
Package: qml-module-qtquick2
Package: qml-module-qtquick-controls2
```

**CMake Detection:**
```cmake
find_package(Qt5 REQUIRED COMPONENTS Core Network Quick Qml)
```

**Actual Location:** `/usr/lib/aarch64-linux-gnu`

#### CommonAPI & vsomeip

**CommonAPI Core Runtime (3.2.x)**
```cmake
find_package(CommonAPI 3.2 REQUIRED)
```

**CommonAPI SomeIP Runtime (3.2.x)**
```cmake
find_package(CommonAPI-SomeIP 3.2 REQUIRED)
```

**vsomeip (3.5.x)**
```cmake
find_package(vsomeip3 3.5 REQUIRED)
```

**Install Location:** `/install_folder/`
- Libraries: `/install_folder/lib/libvsomeip3*.so`
- Headers: `/install_folder/include/`
- CMake configs: `/install_folder/lib/cmake/`

### CommonAPI Generated Code

**Source Location:** `/commonapi/generated/`

**Required Files:**
```
commonapi/generated/
├── core/
│   └── v1/vehiclecontrol/
│       ├── VehicleControl.hpp
│       ├── VehicleControlProxy.hpp
│       ├── VehicleControlProxyBase.hpp
│       └── VehicleControlStub.hpp
└── someip/
    └── v1/vehiclecontrol/
        ├── VehicleControlSomeIPDeployment.cpp
        ├── VehicleControlSomeIPDeployment.hpp
        ├── VehicleControlSomeIPProxy.cpp
        └── VehicleControlSomeIPProxy.hpp
```

**CMake Integration:**
```cmake
set(COMMONAPI_GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../commonapi/generated")
include_directories(
    ${COMMONAPI_GEN_DIR}/core
    ${COMMONAPI_GEN_DIR}/someip
)
```

### Build Tools

```bash
# Standard build tools
cmake (>= 3.16)
g++ (>= 11.4.0, C++17 support required)
make
```

---

## 3. Runtime Dependencies

### vsomeip Configuration Files

Each app requires its own vsomeip configuration:

**BatteryMeter_app:**
```json
// config/vsomeip_batterymeter.json
{
    "applications": [{"name": "BatteryMeter", "id": "0x1002"}],
    "routing": "VehicleControlMock",
    "services": [{"service": "0x1234", "instance": "0x5678"}]
}
```

**GearState_app:**
```json
// config/vsomeip_gearstate.json
{
    "applications": [{"name": "GearState", "id": "0x1004"}],
    "routing": "VehicleControlMock",
    "services": [{"service": "0x1234", "instance": "0x5678"}]
}
```

**Speedometer_app:**
```json
// config/vsomeip_speedometer.json
{
    "applications": [{"name": "Speedometer", "id": "0x1003"}],
    "routing": "VehicleControlMock",
    "services": [{"service": "0x1234", "instance": "0x5678"}]
}
```

### CommonAPI Configuration Files

Each app requires its own CommonAPI configuration:

**Format:**
```ini
# config/commonapi_<appname>.ini
[local]
default=/tmp/commonapi

[segments]
org.genivi.VehicleControl=libvehiclecontrol-service.so
```

### Runtime Libraries (LD_LIBRARY_PATH)

```bash
export LD_LIBRARY_PATH="/install_folder/lib:${LD_LIBRARY_PATH}"
```

**Required .so files:**
- `libvsomeip3.so.3.5.8`
- `libvsomeip3-sd.so.3.5.8`
- `libvsomeip3-cfg.so.3.5.8`
- `libvsomeip3-e2e.so.3.5.8`
- `libCommonAPI.so.3.2.4`
- `libCommonAPI-SomeIP.so.3.2.4`

### Routing Manager

**Binary:** `/deps/vsomeip/build/examples/routingmanagerd/routingmanagerd`

**Required for all vsomeip communication** - must be started before apps

---

## 4. Yocto Recipe Requirements

### meta-middleware Layer Additions

#### New Recipes Needed:

1. **`batterymeter-app_1.0.bb`**
2. **`gearstate-app_1.0.bb`**
3. **`speedometer-app_1.0.bb`**

### Recipe Template

```bitbake
SUMMARY = "IC <AppName> Application"
LICENSE = "CLOSED"

DEPENDS = "qtbase qtdeclarative commonapi commonapi-someip vsomeip3"

SRC_URI = "file://<AppName>_app"

S = "${WORKDIR}/<AppName>_app"

inherit cmake_qt5

EXTRA_OECMAKE = " \
    -DCMAKE_BUILD_TYPE=Release \
    -DCOMMONAPI_GEN_DIR=${STAGING_DIR_TARGET}/usr/include/commonapi \
"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${B}/<AppName>_app ${D}${bindir}/
    
    # Install config files
    install -d ${D}${sysconfdir}/<appname>
    install -m 0644 ${S}/config/vsomeip_<appname>.json ${D}${sysconfdir}/<appname>/
    install -m 0644 ${S}/config/commonapi_<appname>.ini ${D}${sysconfdir}/<appname>/
}

FILES_${PN} += " \
    ${bindir}/<AppName>_app \
    ${sysconfdir}/<appname>/ \
"

RDEPENDS_${PN} = "qtbase qtdeclarative qtquickcontrols2 commonapi commonapi-someip vsomeip3"
```

### Image Recipe Addition

```bitbake
# In meta-instrumentcluster/recipes-core/images/instrument-cluster-image.bb

IMAGE_INSTALL += " \
    batterymeter-app \
    gearstate-app \
    speedometer-app \
    vsomeip3 \
    commonapi \
    commonapi-someip \
"
```

---

## 5. CommonAPI Generated Code Integration

### Option 1: Include in App Source (Current Approach)

**Pros:**
- Self-contained apps
- No separate recipe needed
- Easy to maintain version sync

**Cons:**
- Duplicated code if multiple apps use same interface

**Yocto Recipe:**
```bitbake
SRC_URI = " \
    file://<AppName>_app \
    file://commonapi/generated \
"

EXTRA_OECMAKE = " \
    -DCOMMONAPI_GEN_DIR=${WORKDIR}/commonapi/generated \
"
```

### Option 2: Separate CommonAPI Package (Recommended for Production)

**Pros:**
- Single source of truth
- Easier updates
- Smaller app packages

**Cons:**
- Additional recipe complexity

**New Recipe:** `vehiclecontrol-commonapi_1.0.bb`

```bitbake
SUMMARY = "VehicleControl CommonAPI Generated Interface"
LICENSE = "CLOSED"

SRC_URI = "file://commonapi/generated"

S = "${WORKDIR}/commonapi/generated"

do_install() {
    install -d ${D}${includedir}/commonapi
    cp -r ${S}/* ${D}${includedir}/commonapi/
}

FILES_${PN}-dev = "${includedir}/commonapi"
```

**App Recipe Dependency:**
```bitbake
DEPENDS = "vehiclecontrol-commonapi qtbase qtdeclarative commonapi commonapi-someip vsomeip3"
```

---

## 6. Testing Requirements

### Test Script Created

**Location:** `/app/config/test_ic_apps.sh`

**Functionality:**
1. Check if all apps are built
2. Start vsomeip routing manager
3. Start all three IC apps with proper configs
4. Monitor processes
5. Provide log inspection instructions

**Usage:**
```bash
# Build all IC apps
./build_all_ic_apps.sh

# Test runtime
./test_ic_apps.sh

# Monitor continuously
./test_ic_apps.sh monitor
```

### Expected Test Results

**✅ Visual Verification:**
- [ ] BatteryMeter window appears
- [ ] GearState window appears
- [ ] Speedometer window appears

**✅ vsomeip Communication:**
- [ ] Routing manager starts successfully
- [ ] Apps register with routing manager
- [ ] Service discovery completes (check logs)
- [ ] Apps can send/receive vsomeip messages

**Log Files:**
```bash
/app/BatteryMeter_app/test_run.log
/app/GearState_app/test_run.log
/app/Speedometer_app/test_run.log
```

---

## 7. Known Issues & Solutions

### Issue 1: Qt5 Path Hardcoded in CMakeLists.txt

**Problem:**
```cmake
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};$ENV{HOME}/Qt/5.15.2/gcc_64")
```

**Solution for Yocto:**
Remove custom Qt path, rely on system Qt:
```cmake
# For Yocto, Qt is provided by qtbase package
find_package(Qt5 REQUIRED COMPONENTS Core Network Quick Qml)
```

**Action:** Update CMakeLists.txt before Yocto integration

### Issue 2: DEPLOY_PREFIX Environment Variable

**Current:**
```cmake
if(DEFINED ENV{DEPLOY_PREFIX})
    set(INSTALL_PREFIX $ENV{DEPLOY_PREFIX})
else()
    set(INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../../install_folder")
endif()
```

**Yocto Integration:**
```cmake
# Use standard Yocto paths
set(CMAKE_INSTALL_PREFIX ${D})
set(CMAKE_PREFIX_PATH ${STAGING_DIR_TARGET}/usr)
```

---

## 8. Next Steps

### Immediate (Before Yocto Build)

1. **Test Runtime Functionality**
   ```bash
   cd /app/config
   ./test_ic_apps.sh
   ```

2. **Verify vsomeip Communication**
   - Check logs for service registration
   - Verify message exchange between apps and VehicleControlMock

3. **Document Dependencies for Teammate**
   - Share this document
   - Coordinate CommonAPI interface updates

### Yocto Integration (After Testing)

1. **Clean up CMakeLists.txt**
   - Remove hardcoded Qt paths
   - Use standard CMake variables
   - Add install rules

2. **Create Yocto Recipes**
   - `batterymeter-app_1.0.bb`
   - `gearstate-app_1.0.bb`
   - `speedometer-app_1.0.bb`
   - (Optional) `vehiclecontrol-commonapi_1.0.bb`

3. **Update meta-instrumentcluster Layer**
   - Add recipes to `meta-instrumentcluster/recipes-ic/`
   - Update image recipe

4. **Test Yocto Build**
   ```bash
   bitbake instrument-cluster-image
   ```

---

## 9. Dependency Summary for Yocto JETSON_YOCTO_BUILD_PLAN.md

### Packages to Add

**Build-time:**
- `qtbase-dev`
- `qtdeclarative-dev`
- `qtquickcontrols2-dev`
- `commonapi` (already planned)
- `commonapi-someip` (already planned)
- `vsomeip3` (already planned)

**Runtime:**
- `qtbase`
- `qtdeclarative`
- `qtquickcontrols2`
- `qml-module-qtquick2`
- `qml-module-qtquick-controls2`
- `commonapi`
- `commonapi-someip`
- `vsomeip3`

### New Recipes

```
meta-instrumentcluster/recipes-ic/
├── batterymeter-app/
│   └── batterymeter-app_1.0.bb
├── gearstate-app/
│   └── gearstate-app_1.0.bb
└── speedometer-app/
    └── speedometer-app_1.0.bb
```

### Image Recipe Update

```bitbake
# meta-instrumentcluster/recipes-core/images/instrument-cluster-image.bb
IMAGE_INSTALL += " \
    batterymeter-app \
    gearstate-app \
    speedometer-app \
"
```

---

## 10. Build Script Reference

**Created Scripts:**

1. `/app/BatteryMeter_app/build.sh` - Build BatteryMeter app
2. `/app/GearState_app/build.sh` - Build GearState app
3. `/app/Speedometer_app/build.sh` - Build Speedometer app
4. `/app/config/build_all_ic_apps.sh` - Build all IC apps at once
5. `/app/config/test_ic_apps.sh` - Test runner with vsomeip

**Make executable:**
```bash
chmod +x /app/*/build.sh /app/config/*.sh
```

---

**Status:** ✅ All IC apps built successfully and tested  
**Runtime Status:** ✅ All 3 IC apps run correctly with vsomeip communication  

## Test Results (2026-01-20)

### Build Status
- ✅ BatteryMeter_app: Built successfully
- ✅ GearState_app: Built successfully  
- ✅ Speedometer_app: Built successfully

### Runtime Status
- ✅ BatteryMeter_app: Runs and displays correctly
- ✅ GearState_app: Runs and displays correctly
- ✅ Speedometer_app: Runs and displays correctly
- ✅ vsomeip communication: Working

### Key Findings

1. **Wayland Display**: IC apps must connect to `wayland-0` (Weston directly), NOT `wayland-1` (HU compositor)
2. **Routing Manager**: Must be started separately before both HU and IC apps
3. **Permissions**: Apps need `sudo` to access Wayland socket at `/tmp/xdg/wayland-0`

### Test Scripts

**Build all IC apps:**
```bash
cd /home/jetson/leo/DES_Head-Unit/app/config
./build_all_ic_apps.sh
```

**Start routing manager (once):**
```bash
cd /home/jetson/leo/DES_Head-Unit/app/config
./start_routing_manager.sh
```

**Test IC apps:**
```bash
# Prerequisites: 
#   1. Weston must be running (./run-jetson-wayland-full.sh)
#   2. Routing manager must be running (./start_routing_manager.sh)

cd /home/jetson/leo/DES_Head-Unit/app/config
./test_ic_apps.sh
```

**Run HU + IC together:**
```bash
# Terminal 1: Start Weston + Routing Manager + HU Apps
sudo ./run-jetson-wayland-full.sh

# Terminal 2: Start IC Apps
cd app/config
./test_ic_apps.sh
```

---

**Next Action:** Update Yocto build plan with confirmed dependencies and Wayland configuration

