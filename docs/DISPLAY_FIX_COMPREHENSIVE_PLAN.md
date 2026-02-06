# Jetson Orin Nano 디스플레이 신호 문제 해결 종합 계획

**작성일**: 2026-01-30  
**목표**: "Input signal out of range" 문제 해결 및 안정적인 1920x1080@60Hz 화면 출력

---

## 📋 현재 상황 분석

### 1. 환경 구성 현황

**위치**: `~/yocto-jetson/tegra-demo-distro/`

```
구조:
├── layers/
│   ├── meta-tegra              (OE4T 공식 BSP)
│   ├── meta-tegrademo          (OE4T 데모 이미지 레시피)
│   ├── meta-jetson-headunit    (커스텀: weston.ini 포함)
│   └── meta-middleware         (커스텀: CommonAPI 등)
├── build-headunit/             (활성 빌드 디렉토리)
└── setup-env                   (OE4T 표준 환경 스크립트)
```

**빌드 환경**:
- **Repository**: OE4T/tegra-demo-distro (공식)
- **MACHINE**: jetson-orin-nano-devkit
- **DISTRO**: tegrademo
- **활성 레이어**: 14개 (표준 OE4T + 2개 커스텀)

### 2. DISTRO_FEATURES 확인 (현재 빌드)

```bash
DISTRO_FEATURES="wayland opengl x11 systemd pulseaudio vulkan virtualization ..."
```

✅ **모든 필수 기능 이미 활성화됨**:
- `wayland` - Wayland 컴포지터 지원
- `opengl` - GPU 가속 그래픽
- `x11` - X11 지원 (demo-image-full용)
- `virtualization` - 컨테이너 지원

### 3. 사용 가능한 이미지 레시피

| 이미지 | 필수 기능 | 크기 예상 | 설명 |
|--------|----------|---------|------|
| **demo-image-egl** | `opengl` | ~500MB | GPU/EGL만 (GUI 없음) |
| **demo-image-weston** | `wayland opengl` | ~600MB | Wayland + Weston (현재 사용 중) |
| **demo-image-full** | `x11 opengl virtualization` | ~2GB | X11/Sato + CUDA + TensorRT |

### 4. 커스텀 weston.ini 확인

**위치**: `layers/meta-jetson-headunit/recipes-graphics/weston/files/weston.ini`

```ini
[core]
backend=drm-backend.so
require-input=false
idle-time=0
gbm-format=rgb565

[output]
name=DP-1
mode=1920x1080@60        # ✅ 올바른 설정
transform=normal

[shell]
panel-position=none
locking=false
background-color=0xff002244  # 파란 배경 (테스트용)
```

**배포 메커니즘**: `weston-init.bbappend`를 통해 `/etc/xdg/weston/weston.ini`에 자동 설치됨

### 5. 이전 시도 이력

1. **demo-image-weston + SSH 추가 빌드** (2026-01-29)
   - 결과: 617MB tegraflash
   - 상태: 플래시 성공, 네트워크 연결 확인
   - 문제: SSH 접속 실패 (포트 22 닫힘)

2. **SSH 서비스 활성화 재시도** (2026-01-30)
   - 수정: `SYSTEMD_AUTO_ENABLE:pn-openssh = "enable"` 추가
   - 상태: rootfs 재빌드 완료
   - 문제: **GPT 초기화 실패로 플래시 불가**

---

## 🔍 제시된 가이드 비판적 검토

### OE4T 가이드 vs 현재 환경

| 항목 | 가이드 권장 | 현재 상태 | 평가 |
|------|-----------|----------|------|
| **저장소** | OE4T/tegra-demo-distro | ✅ 동일 | 정석 |
| **setup-env 사용** | `source ./setup-env --machine ...` | ✅ 사용됨 | 정석 |
| **DISTRO_FEATURES** | `wayland x11 opengl` 추가 | ✅ 이미 포함 | 완료 |
| **이미지 선택** | demo-image-full 권장 | ❌ demo-image-weston 사용 중 | 논쟁 여지 있음 |
| **플래시 방법** | `sudo ./doflash.sh` | ✅ initrd-flash 사용 (더 고급) | 정석 |

### 핵심 질문: demo-image-full이 필요한가?

**demo-image-full의 장점**:
- ✅ X11/Sato GUI 포함 (완전한 데스크톱 환경)
- ✅ CUDA, TensorRT, VPI 샘플 포함
- ✅ nvidia-docker 포함
- ✅ "검증된" 풀스택 이미지

**demo-image-full의 단점**:
- ❌ 빌드 시간 **4-6시간** (demo-weston의 3-4배)
- ❌ 이미지 크기 **~2GB** (demo-weston의 3배)
- ❌ X11 종속성 (프로젝트는 Wayland 기반)
- ❌ 불필요한 패키지 다수 (AI 프레임워크 등)

**demo-image-weston의 장점**:
- ✅ Wayland 네이티브 (현대적)
- ✅ 빌드 시간 **1-2시간**
- ✅ 이미지 크기 **~600MB**
- ✅ 프로젝트 요구사항 충족 (CommonAPI/Qt 앱 실행)
- ✅ **weston.ini가 이미 올바르게 구성됨**

### 결론: demo-image-weston이 더 적합

**이유**:
1. 프로젝트는 **Wayland 기반 Qt 앱** 개발 중
2. weston.ini가 **이미 올바르게 설정됨** (1920x1080@60)
3. 빌드 시간/리소스 효율성
4. demo-image-full의 X11은 **프로젝트에 불필요**

**문제의 핵심은 이미지 선택이 아님**:
- weston.ini 설정은 올바름
- 빌드 환경도 정석적
- **SSH 접속 실패로 검증 불가능한 상태**

---

## 🎯 해결 전략 3가지 옵션

### 옵션 A: 현재 빌드 플래시 재시도 (최단 시간)

**상황**: SSH 포함 demo-image-weston 빌드 완료, GPT 오류로 플래시 실패

**장점**:
- ✅ 빌드 완료 (추가 시간 불필요)
- ✅ weston.ini 포함 확인됨
- ✅ SSH 포함 (검증 가능)

**단점**:
- ❌ GPT 오류 원인 불명
- ❌ SSH 서비스 시작 보장 불확실

**예상 시간**: 30분-1시간 (하드웨어 리셋 + 재플래시)

**단계**:
1. Jetson 완전 전원 차단 (30초 대기)
2. Recovery 모드 재진입
3. `sudo ./doflash.sh` 사용 (initrd-flash 대신)
4. 플래시 성공 시 SSH 접속 시도
5. SSH 성공 → weston.ini 확인
6. SSH 실패 → 옵션 B로 전환

**위험도**: 중 (SSH 서비스 시작 보장 없음)

---

### 옵션 B: demo-image-weston 클린 재빌드 (중간 시간)

**상황**: 커스텀 레이어는 유지하되, SSH 설정 정리하고 재빌드

**장점**:
- ✅ weston.ini 보존 (meta-jetson-headunit)
- ✅ SSH 설정 검증 및 최적화
- ✅ 빌드 캐시 활용 (부분 재빌드)
- ✅ 가장 **균형 잡힌 접근**

**단점**:
- ⏱️ 재빌드 시간 1-2시간

**예상 시간**: 2-3시간 (재빌드 + 플래시 + 검증)

**단계**:
1. **local.conf 정리**:
   ```bash
   # SSH 관련 설정 최적화
   EXTRA_IMAGE_FEATURES:append = " ssh-server-openssh"
   
   # systemd-networkd 명시적 활성화
   DISTRO_FEATURES:append = " systemd-networkd"
   
   # 디버깅용 직렬 콘솔 활성화 (SSH 실패 대비)
   EXTRA_IMAGE_FEATURES:append = " serial-autologin-root"
   ```

2. **클린 재빌드**:
   ```bash
   cd ~/yocto-jetson/tegra-demo-distro/build-headunit
   bitbake demo-image-weston -c cleansstate
   bitbake demo-image-weston
   ```

3. **플래시**:
   ```bash
   cd ~ && rm -rf jetson-flash && mkdir jetson-flash
   cd jetson-flash
   tar xzf ~/yocto-jetson/.../demo-image-weston-*.tegraflash.tar.gz
   sudo ./doflash.sh
   ```

4. **검증**:
   - HDMI 신호 확인 (1920x1080@60)
   - SSH 접속: `ssh root@192.168.1.101`
   - weston.ini 확인: `cat /etc/xdg/weston/weston.ini`
   - Weston 로그: `journalctl -u weston`

**위험도**: 낮 (검증된 방법)

---

### 옵션 C: demo-image-egl 최소 빌드 (가장 빠름)

**상황**: GUI 없는 EGL만으로 디스플레이 신호 테스트

**장점**:
- ✅ 가장 빠른 빌드 (~30분)
- ✅ 최소 종속성 (오류 가능성 최소)
- ✅ SSH 포함 가능
- ✅ DRM/KMS 레벨 테스트 가능

**단점**:
- ❌ Weston 없음 (GUI 테스트 불가)
- ❌ 최종 목표 환경 아님
- ❌ 추가 빌드 필요 (검증 후 Weston으로 전환)

**예상 시간**: 1-1.5시간 (빌드 + 플래시 + 검증)

**단계**:
1. **demo-image-egl 빌드**:
   ```bash
   cd ~/yocto-jetson/tegra-demo-distro/build-headunit
   # local.conf에 SSH 설정 유지
   bitbake demo-image-egl
   ```

2. **플래시 및 검증**:
   ```bash
   # SSH 접속 후
   dmesg | grep -i drm
   cat /sys/class/drm/card*/status
   modetest -M tegra-drm -c
   ```

3. **신호 확인 후 Weston 빌드**:
   ```bash
   bitbake demo-image-weston  # 대부분 캐시 재사용
   ```

**위험도**: 낮 (진단 목적으로 유용)

---

## 📊 옵션 비교표

| 기준 | 옵션 A (재플래시) | 옵션 B (재빌드) | 옵션 C (EGL) |
|------|-----------------|----------------|--------------|
| **예상 시간** | 30분-1시간 | 2-3시간 | 1-1.5시간 |
| **성공 확률** | 60% | 85% | 75% (진단용) |
| **위험도** | 중 | 낮 | 낮 |
| **검증 가능성** | SSH 불확실 | SSH 확실 | SSH 확실 |
| **최종 목표 도달** | 1단계 | 1단계 | 2단계 |
| **디버깅 용이성** | 낮 | 높음 | 매우 높음 |

---

## 💡 최종 권장 전략

### **1순위: 옵션 B (demo-image-weston 클린 재빌드)**

**이유**:
1. **정확성 최우선**: 사용자 요구사항
2. **검증 가능**: SSH + serial-autologin 모두 활성화
3. **균형**: 시간(2-3시간) vs 성공률(85%)
4. **유지보수**: 커스텀 레이어 보존, 향후 앱 통합 용이
5. **문제 격리**: SSH 설정을 명확히 정리

### **2순위: 옵션 A (현재 빌드 재플래시)**

**조건부 사용**:
- 옵션 B 빌드 시작 전에 **한 번 더 시도**
- `doflash.sh` 사용 (initrd-flash보다 단순)
- 실패 시 즉시 옵션 B로 전환

### **3순위: 옵션 C (demo-image-egl)**

**사용 시나리오**:
- 옵션 B 실패 시 디버깅 목적
- DRM/KMS 레벨 문제 의심 시
- 하드웨어 신호 출력 자체를 검증

---

## 🚀 실행 계획 (옵션 B 기준)

### Phase 1: 환경 준비 (5분)

```bash
# 1. 빌드 환경 활성화
cd ~/yocto-jetson/tegra-demo-distro
source layers/oe-init-build-env build-headunit

# 2. 현재 설정 백업
cp conf/local.conf conf/local.conf.backup-$(date +%Y%m%d-%H%M%S)

# 3. 디스크 공간 확인
df -h /home
# 필요: 최소 50GB 여유
```

### Phase 2: local.conf 최적화 (10분)

```bash
# 4. local.conf 수정
cat >> conf/local.conf << 'EOF'

#
# === 디스플레이 문제 해결을 위한 최적화 설정 ===
#

# SSH 서비스 (검증용)
EXTRA_IMAGE_FEATURES:append = " ssh-server-openssh"
EXTRA_IMAGE_FEATURES:append = " serial-autologin-root"

# 네트워킹 (systemd-networkd)
DISTRO_FEATURES:append = " systemd-networkd"

# 디버깅 심볼 포함 (문제 분석용)
EXTRA_IMAGE_FEATURES:append = " dbg-pkgs"

# 빌드 성능 최적화
BB_NUMBER_THREADS = "8"
PARALLEL_MAKE = "-j 6"

# SSH 강제 활성화
SYSTEMD_AUTO_ENABLE:pn-openssh = "enable"

EOF

# 5. 설정 검증
grep -A 20 "디스플레이 문제" conf/local.conf
```

### Phase 3: 클린 재빌드 (1-2시간)

```bash
# 6. 이전 빌드 상태 정리
bitbake demo-image-weston -c cleansstate

# 7. 재빌드 시작
bitbake demo-image-weston 2>&1 | tee ~/demo-weston-clean-rebuild-$(date +%Y%m%d-%H%M%S).log

# 8. 빌드 완료 확인
ls -lh tmp/deploy/images/jetson-orin-nano-devkit/demo-image-weston*.tegraflash.tar.gz
```

**예상 시간**: 1-2시간  
**완료 조건**: "NOTE: Tasks Summary: Attempted XXXX tasks of which XXXX didn't need to be rerun"

### Phase 4: 플래시 준비 (5분)

```bash
# 9. 플래시 디렉토리 준비
cd ~
rm -rf jetson-flash
mkdir jetson-flash
cd jetson-flash

# 10. 이미지 압축 해제
tar xzf ~/yocto-jetson/tegra-demo-distro/build-headunit/tmp/deploy/images/jetson-orin-nano-devkit/demo-image-weston-jetson-orin-nano-devkit.tegraflash.tar.gz

# 11. 스크립트 권한 확인
chmod +x doflash.sh initrd-flash
ls -lh doflash.sh initrd-flash
```

### Phase 5: Jetson 플래시 (10-15분)

```bash
# 12. Jetson Recovery 모드 진입
# 하드웨어 작업:
# - FC REC와 GND 연결 (점퍼/케이블)
# - 전원 OFF → 30초 대기 → 전원 ON
# - FC REC 연결 해제

# 13. Recovery 모드 확인
lsusb | grep -i nvidia
# 출력 예상: "NVidia Corp. APX"

# 14. 플래시 실행 (doflash 사용 - GPT 문제 회피)
sudo ./doflash.sh

# 15. 플래시 완료 대기 (10-15분)
# 성공 메시지 대기:
# "*** The target t186ref has been flashed successfully. ***"
```

### Phase 6: 검증 (10분)

```bash
# 16. 네트워크 연결 확인
ping -c 3 192.168.1.101

# 17. SSH 접속
ssh root@192.168.1.101
# (비밀번호 없음 - debug-tweaks 활성화)

# Jetson에서 실행:

# 18. weston.ini 확인
cat /etc/xdg/weston/weston.ini
# 예상: [output] name=DP-1, mode=1920x1080@60

# 19. Weston 서비스 상태
systemctl status weston
journalctl -u weston -n 50

# 20. DRM 출력 확인
cat /sys/class/drm/card0-DP-1/status
# 예상: "connected"

cat /sys/class/drm/card0-DP-1/modes
# 예상: "1920x1080" 포함

# 21. 모니터 EDID 확인
modetest -M tegra-drm -c | grep -A 5 "^Connectors"

# 22. Weston 프로세스 확인
ps aux | grep weston
```

### Phase 7: 문제 해결 시나리오

#### 시나리오 A: HDMI 신호 여전히 없음

```bash
# Jetson에서:
# 1. Weston 로그 확인
journalctl -u weston | tail -50

# 2. DRM 디바이스 확인
ls -l /dev/dri/
dmesg | grep -i drm | tail -20

# 3. Weston 수동 재시작
systemctl restart weston
sleep 5
systemctl status weston

# 4. 수동 모드 설정 테스트
export WAYLAND_DISPLAY=wayland-0
weston --backend=drm-backend.so --config=/etc/xdg/weston/weston.ini &
```

#### 시나리오 B: SSH 접속 실패

```bash
# 호스트에서:
# 1. 직렬 콘솔 사용 (USB-TTL 케이블 필요)
# 또는 HDMI가 작동하면 키보드로 직접 접근

# 2. SSH 서비스 확인
systemctl status sshd
systemctl status sshd.socket

# 3. 네트워크 확인
ip addr show
systemctl status systemd-networkd

# 4. SSH 수동 시작
systemctl start sshd
systemctl enable sshd
```

#### 시나리오 C: 빌드 실패

```bash
# 1. 로그 확인
tail -100 ~/demo-weston-clean-rebuild-*.log

# 2. 개별 패키지 재빌드
bitbake <failed-package> -c cleansstate
bitbake <failed-package>

# 3. 캐시 정리 (최후 수단)
rm -rf tmp/cache
bitbake demo-image-weston
```

---

## ⚠️ 위험 요소 및 대응

| 위험 | 확률 | 영향 | 대응 방안 |
|------|-----|------|---------|
| **빌드 실패** | 낮음 (10%) | 높음 | 로그 분석 → 개별 패키지 재빌드 |
| **디스크 부족** | 낮음 (15%) | 높음 | tmp/ 정리, 불필요 파일 삭제 |
| **플래시 GPT 오류** | 중간 (40%) | 중간 | doflash.sh 사용, Jetson 완전 리셋 |
| **SSH 여전히 실패** | 낮음 (20%) | 중간 | serial-autologin으로 직접 접근 |
| **HDMI 신호 여전히 없음** | 중간 (30%) | 높음 | DRM 로그 분석, 하드웨어 케이블 교체 |

---

## 📈 성공 기준

### 필수 (MUST)
- ✅ Jetson이 부팅되고 네트워크 연결됨
- ✅ SSH 또는 serial console로 접근 가능
- ✅ `/etc/xdg/weston/weston.ini` 존재 및 내용 정확
- ✅ Weston 서비스가 실행 중 (`systemctl status weston`)

### 목표 (SHOULD)
- ✅ HDMI 모니터에 1920x1080@60Hz 신호 출력
- ✅ 파란 배경 (0xff002244) 표시
- ✅ Weston 터미널 실행 가능

### 이상적 (NICE-TO-HAVE)
- ✅ CommonAPI 앱 실행 가능
- ✅ Qt 앱 Weston에서 실행 가능

---

## 📝 예상 타임라인

| 단계 | 시작 | 예상 소요 | 누적 시간 |
|------|------|----------|----------|
| 환경 준비 | T+0 | 5분 | 0:05 |
| local.conf 최적화 | T+0:05 | 10분 | 0:15 |
| 클린 재빌드 | T+0:15 | 90분 | 1:45 |
| 플래시 준비 | T+1:45 | 5분 | 1:50 |
| Jetson 플래시 | T+1:50 | 15분 | 2:05 |
| 검증 | T+2:05 | 10분 | 2:15 |
| **버퍼 (문제 해결)** | - | 45분 | **3:00** |

**총 예상 시간: 2시간 15분 - 3시간**

---

## 🎓 학습 사항 및 향후 개선

### 이번 문제의 교훈

1. **최소 이미지의 함정**: demo-image-weston은 최소 구성 → SSH 등 수동 추가 필요
2. **검증 메커니즘 필수**: 원격 접근 없이 디버깅 불가능
3. **Flash 방법 차이**: initrd-flash vs doflash.sh - GPT 오류 발생 시 후자 시도
4. **systemd 서비스 관리**: socket vs service 차이 이해 필요

### 향후 베스트 프랙티스

1. **커스텀 이미지 레시피 생성**:
   ```bash
   # layers/meta-jetson-headunit/recipes-demo/images/jetson-dev-image.bb
   require recipes-demo/images/demo-image-weston.bb
   
   IMAGE_FEATURES += "ssh-server-openssh serial-autologin-root"
   EXTRA_IMAGE_FEATURES += "tools-debug tools-profile"
   ```

2. **systemd-conf 레이어 개선**:
   - DHCP + 정적 IP fallback
   - SSH 서비스 강제 활성화
   - Weston 자동 재시작 설정

3. **CI/CD 통합**:
   - 빌드 성공 자동 검증
   - 플래시 후 자동 네트워크 테스트
   - HDMI 신호 검증 스크립트

---

## 🔗 참고 자료

- [OE4T tegra-demo-distro](https://github.com/OE4T/tegra-demo-distro)
- [meta-tegra 문서](https://github.com/OE4T/meta-tegra)
- [Weston DRM Backend](https://wayland.freedesktop.org/weston.html)
- [Jetson Orin Nano 개발자 가이드](https://developer.nvidia.com/embedded/jetson-orin-nano-developer-kit)

---

## 결론

**현재 환경은 이미 OE4T 표준 방식을 따르고 있으며**, weston.ini 설정도 올바릅니다. 문제는:

1. ❌ SSH 접속 불가 → 검증 불가능
2. ❌ GPT 오류 → 플래시 불가능

**해결책**:
- ✅ **옵션 B (클린 재빌드)** - SSH 설정 최적화 + serial console 추가
- ✅ `doflash.sh` 사용 - GPT 문제 회피
- ✅ 2-3시간 투자 → **확실한 검증 및 해결**

demo-image-full로의 전환은 **불필요**합니다. 현재 접근 방식이 정석이며, 실행만 필요합니다.
