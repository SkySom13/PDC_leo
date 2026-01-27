# Yocto Recipe 작성 계획 (Jetson Orin Nano)

**작성일**: 2026년 1월 27일  
**목표**: 완성된 HU/IC 앱들을 Jetson Yocto 이미지에 통합

---

## 📋 앱 분류 및 우선순위

### HU (Head Unit) 앱들 - ECU2 (Jetson Orin Nano)
| 앱 | 경로 | 빌드 시스템 | vsomeip | Qt5 | 우선순위 |
|----|------|------------|---------|-----|----------|
| **HomeScreenApp** | `app/HomeScreenApp/` | CMake | ✅ | ✅ | **P0** |
| **MediaApp** | `app/MediaApp/` | CMake | ✅ | ✅ | **P0** |
| **AmbientApp** | `app/AmbientApp/` | CMake | ✅ | ✅ | **P0** |
| **GearApp** | `app/GearApp/` | CMake | ✅ | ✅ | **P0** |
| **HU_MainApp** | `app/HU_MainApp/` | CMake | ❌ | ✅ | P1 (Compositor) |

### IC (Instrument Cluster) 앱들 - 듀얼 디스플레이 준비
| 앱 | 경로 | 빌드 시스템 | vsomeip | Qt5 | 우선순위 |
|----|------|------------|---------|-----|----------|
| **IC_Compositor** | `app/IC_Compositor/` | CMake | ❌ | ✅ | P2 (MST Hub 도착 후) |
| **Speedometer_app** | `app/Speedometer_app/` | CMake | ✅ | ✅ | P2 |
| **GearState_app** | `app/GearState_app/` | CMake | ✅ | ✅ | P2 |
| **BatteryMeter_app** | `app/BatteryMeter_app/` | CMake | ✅ | ✅ | P2 |

### ECU1 (VehicleControl) - Raspberry Pi
| 앱 | 경로 | 빌드 시스템 | CAN | 우선순위 |
|----|------|------------|-----|----------|
| **VehicleControlECU** | `app/VehicleControlECU/` | CMake | ✅ | P3 (별도 빌드) |

---

## 🎯 Phase 1: HU 앱들 (P0 우선순위)

### 1.1 의존성 분석

#### 공통 의존성
```cmake
Qt5 5.15.7:
- Qt5Core
- Qt5Gui
- Qt5Qml
- Qt5Quick
- Qt5Multimedia (MediaApp만)

vsomeip 3.5.8:
- vsomeip3
- Boost 1.74+ (system, thread, filesystem)

CommonAPI 3.2.4:
- CommonAPI-Core
- CommonAPI-SomeIP

Generated Code:
- ../../commonapi/generated/core/
- ../../commonapi/generated/someip/
```

#### 앱별 특징
**HomeScreenApp**:
- MediaControl, AmbientControl 프록시
- QML UI: `qml/main.qml`
- vsomeip 클라이언트

**MediaApp**:
- MediaControl 서비스 제공
- Qt Multimedia 사용
- QML UI + 오디오 재생

**AmbientApp**:
- AmbientControl 서비스 제공
- QML UI

**GearApp**:
- VehicleControl 프록시
- 기어 상태 수신
- QML UI

### 1.2 Recipe 작성 순서

```
1단계: meta-middleware 복사 (vsomeip, CommonAPI)
   ├─> meta-middleware 전체 복사
   └─> bblayers.conf에 추가

2단계: commonapi-generated 패키지 생성
   ├─> recipes-comm/commonapi-generated/
   ├─> ../../commonapi/generated/ 소스 복사
   └─> .bb 파일 작성

3단계: HU 앱 Recipe 작성 (4개)
   ├─> recipes-apps/homescreenapp/homescreenapp_1.0.bb
   ├─> recipes-apps/mediaapp/mediaapp_1.0.bb
   ├─> recipes-apps/ambientapp/ambientapp_1.0.bb
   └─> recipes-apps/gearapp/gearapp_1.0.bb

4단계: systemd 서비스 작성
   ├─> recipes-apps/*/files/*.service
   ├─> vsomeip-routingmanager.service
   └─> weston.service 설정

5단계: Weston 설정
   ├─> recipes-graphics/weston/weston_%.bbappend
   └─> files/weston-jetson.ini

6단계: 이미지 Recipe 업데이트
   ├─> jetson-headunit-image.bb
   └─> IMAGE_INSTALL += 앱들 추가
```

---

## 📝 Recipe Template

### GearApp 예시

**파일**: `meta-jetson-headunit/recipes-apps/gearapp/gearapp_1.0.bb`

```bitbake
SUMMARY = "Gear Selection Application for Jetson HU"
DESCRIPTION = "Qt5/QML gear selector with vsomeip communication"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "qtbase qtdeclarative qtquickcontrols2 vsomeip commonapi-core commonapi-someip commonapi-generated"
RDEPENDS:${PN} = "qtwayland qtgraphicaleffects qml-module-qtquick-controls2 vsomeip"

SRC_URI = " \
    file://CMakeLists.txt \
    file://src/ \
    file://qml/ \
    file://qml.qrc \
    file://run.sh \
    file://gearapp.service \
"

S = "${WORKDIR}"

inherit cmake_qt5 systemd

EXTRA_OECMAKE = " \
    -DCMAKE_BUILD_TYPE=Release \
    -DCOMMONAPI_GENERATED_DIR=${STAGING_DIR_HOST}${includedir}/commonapi-generated \
"

SYSTEMD_SERVICE:${PN} = "gearapp.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install:append() {
    # 실행 파일
    install -d ${D}${bindir}
    install -m 0755 ${B}/GearApp ${D}${bindir}/

    # QML 리소스
    install -d ${D}${datadir}/gearapp/qml
    cp -r ${S}/qml/* ${D}${datadir}/gearapp/qml/

    # systemd service
    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${S}/gearapp.service ${D}${systemd_unitdir}/system/

    # 실행 스크립트
    install -m 0755 ${S}/run.sh ${D}${bindir}/gearapp-run
}

FILES:${PN} += " \
    ${bindir}/GearApp \
    ${bindir}/gearapp-run \
    ${datadir}/gearapp/ \
"
```

**systemd service**: `files/gearapp.service`

```ini
[Unit]
Description=Gear Selection App
Requires=weston.service vsomeip-routingmanager.service
After=weston.service vsomeip-routingmanager.service network-online.target
Wants=network-online.target

[Service]
Type=simple
User=weston
Group=weston
Environment="WAYLAND_DISPLAY=wayland-0"
Environment="XDG_RUNTIME_DIR=/run/user/1000"
Environment="QT_QPA_PLATFORM=wayland"
Environment="LD_LIBRARY_PATH=/usr/lib"
Environment="VSOMEIP_CONFIGURATION=/etc/vsomeip/routing_manager_ecu2.json"
Environment="COMMONAPI_CONFIG=/usr/share/commonapi/commonapi.ini"
ExecStart=/usr/bin/GearApp
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

---

## 🔧 빌드 환경 설정

### bblayers.conf 수정
```bash
BBLAYERS ?= " \
  /home/seame/yocto-jetson/poky/meta \
  /home/seame/yocto-jetson/poky/meta-poky \
  /home/seame/yocto-jetson/meta-tegra \
  /home/seame/yocto-jetson/meta-openembedded/meta-oe \
  /home/seame/yocto-jetson/meta-openembedded/meta-python \
  /home/seame/yocto-jetson/meta-openembedded/meta-networking \
  /home/seame/yocto-jetson/meta-qt5 \
  /home/seame/yocto-jetson/meta-middleware \
  /home/seame/yocto-jetson/meta-jetson-headunit \
"
```

### local.conf 추가 설정
```bash
# Boost (vsomeip 의존성)
PREFERRED_VERSION_boost = "1.74%"

# vsomeip
PREFERRED_VERSION_vsomeip = "3.5.8"

# CommonAPI
PREFERRED_VERSION_commonapi-core = "3.2.4"
PREFERRED_VERSION_commonapi-someip = "3.2.4"

# 네트워크 (vsomeip 멀티캐스트)
PACKAGECONFIG:append:pn-systemd = " networkd resolved"
```

---

## 📦 Phase별 구현 계획

### Phase 1.1: meta-middleware 통합 (1일)
```bash
✓ meta-middleware를 yocto-jetson/에 복사
✓ bblayers.conf 업데이트
✓ 테스트 빌드: bitbake vsomeip commonapi-core commonapi-someip
```

### Phase 1.2: commonapi-generated 패키지 (1일)
```bash
✓ recipes-comm/commonapi-generated/ 생성
✓ ../../commonapi/generated/ 소스 복사
✓ commonapi-generated_1.0.bb 작성
✓ 테스트 빌드: bitbake commonapi-generated
```

### Phase 1.3: GearApp Recipe (1일)
```bash
✓ recipes-apps/gearapp/ 생성
✓ 소스 복사 (CMakeLists.txt, src/, qml/)
✓ gearapp_1.0.bb 작성
✓ systemd service 작성
✓ 테스트 빌드: bitbake gearapp
```

### Phase 1.4: 나머지 HU 앱 Recipe (2일)
```bash
✓ MediaApp Recipe
✓ AmbientApp Recipe
✓ HomeScreenApp Recipe
✓ 개별 테스트 빌드
```

### Phase 1.5: 이미지 통합 및 플래싱 (1일)
```bash
✓ jetson-headunit-image.bb 업데이트
✓ 전체 빌드: bitbake jetson-headunit-image
✓ 플래싱 테스트
✓ 부팅 및 동작 검증
```

---

## ✅ 검증 체크리스트

### 빌드 검증
- [ ] vsomeip 빌드 성공
- [ ] commonapi 빌드 성공
- [ ] commonapi-generated 빌드 성공
- [ ] GearApp 빌드 성공
- [ ] MediaApp 빌드 성공
- [ ] AmbientApp 빌드 성공
- [ ] HomeScreenApp 빌드 성공
- [ ] jetson-headunit-image 빌드 성공 (에러 없음)

### 플래싱 검증
- [ ] tegraflash 패키지 생성 확인
- [ ] 플래싱 성공 (ERR 메시지 없음)
- [ ] Jetson 부팅 성공

### 런타임 검증
```bash
# Jetson SSH 접속 후
ps aux | grep weston          # Weston 실행 확인
ps aux | grep vsomeip         # Routing Manager 실행 확인
ps aux | grep -E "GearApp|MediaApp|AmbientApp|HomeScreen"  # 앱들 실행 확인
systemctl status weston
systemctl status vsomeip-routingmanager
systemctl status gearapp
systemctl status mediaapp
systemctl status ambientapp
systemctl status homescreenapp
journalctl -u gearapp -f      # 로그 확인
```

### 기능 검증
- [ ] Weston 화면 출력 확인 (HDMI/DP-1)
- [ ] Qt5 앱 UI 표시 확인
- [ ] vsomeip 통신 확인 (앱 간 메시지 전달)
- [ ] 터치/마우스 입력 반응 확인

---

## 🚀 다음 Phase (Phase 2: IC 앱들)

MST Hub 도착 후:
1. IC_Compositor Recipe 작성
2. Speedometer_app, GearState_app, BatteryMeter_app Recipe 작성
3. 듀얼 디스플레이 Weston 설정
4. 통합 테스트

---

## 📚 참고 문서

- `/home/seame/leo/DES_Head-Unit/docs/JETSON_YOCTO_BUILD_PLAN.md`
- `/home/seame/leo/DES_Head-Unit/app/config/README.md`
- `/home/seame/yocto-jetson/meta-middleware/`
- Yocto Manual: https://docs.yoctoproject.org/

---

## 예상 소요 시간

| Phase | 작업 | 소요 시간 |
|-------|------|----------|
| 1.1 | meta-middleware 통합 | 1일 |
| 1.2 | commonapi-generated | 1일 |
| 1.3 | GearApp Recipe | 1일 |
| 1.4 | 나머지 3개 앱 | 2일 |
| 1.5 | 이미지 통합 & 테스트 | 1일 |
| **총합** | **Phase 1 완료** | **6일** |

**빌드 시간**: 첫 빌드 6-12시간, 증분 빌드 30분~2시간
