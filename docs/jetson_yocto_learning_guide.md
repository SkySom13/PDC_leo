# Jetson Yocto 학습 가이드

## 📖 공식 문서 (필수)

### 1. OE4T (OpenEmbedded for Tegra) - 가장 중요
**저장소**: https://github.com/OE4T/meta-tegra
**문서**: https://github.com/OE4T/meta-tegra/wiki

**핵심 페이지:**
- Getting Started: https://github.com/OE4T/meta-tegra/wiki/Getting-Started
- Layer Contents: https://github.com/OE4T/meta-tegra/wiki/Layer-Contents
- Flashing Guide: https://github.com/OE4T/meta-tegra/wiki/Flashing

### 2. tegra-demo-distro (실전 예제)
**저장소**: https://github.com/OE4T/tegra-demo-distro
**README**: 단계별 빌드 가이드 포함

### 3. NVIDIA 공식 문서
**Jetson Linux (L4T)**: https://developer.nvidia.com/embedded/jetson-linux
**Developer Guide**: https://docs.nvidia.com/jetson/

## 🚀 추천 학습 순서 (2주 계획)

### Week 1: 기본기 다지기

**Day 1-2: 환경 이해**
- [ ] tegra-demo-distro README 정독
- [ ] demo-image-minimal 빌드 및 플래시
- [ ] 목표: 부팅만 되는 최소 이미지

**Day 3-4: Layer 구조 파악**
- [ ] meta-tegra/recipes-bsp/ 탐색 (부트로더, 커널)
- [ ] meta-tegra/recipes-graphics/ 탐색 (Weston, X11)
- [ ] 목표: BSP 레이어 구조 이해

**Day 5-7: 커스터마이징**
- [ ] local.conf에서 MACHINE, DISTRO_FEATURES 변경
- [ ] demo-image-weston 빌드
- [ ] 목표: GUI 띄우기 성공

### Week 2: 실전 적용

**Day 8-10: 멀티미디어 및 Qt**
- [ ] meta-qt5 레이어 추가
- [ ] Qt 애플리케이션 빌드 및 실행
- [ ] GStreamer + NVENC 테스트

**Day 11-12: 커스텀 레시피 작성**
- [ ] 자체 .bb 파일 작성
- [ ] 커스텀 systemd 서비스 추가
- [ ] 목표: 부팅 시 자동 실행

**Day 13-14: 최적화**
- [ ] 이미지 크기 최소화
- [ ] 부팅 시간 단축
- [ ] 목표: 프로덕션 준비

## 📝 실습 예제 (지금 바로 가능)

### 예제 1: 최소 이미지 빌드 (30분)
```bash
cd ~/yocto-jetson/tegra-demo-distro
source layers/oe-init-build-env build-minimal

# conf/local.conf 편집
echo 'MACHINE = "jetson-orin-nano-devkit"' > conf/local.conf

# 빌드
bitbake core-image-minimal

# 플래시
cd tmp/deploy/images/jetson-orin-nano-devkit/
tar xzf core-image-minimal-*.tegraflash.tar.gz
sudo ./doflash.sh
```

### 예제 2: SSH 추가 (10분)
```bash
# conf/local.conf에 추가
cat >> conf/local.conf << 'EOF'
EXTRA_IMAGE_FEATURES:append = " ssh-server-openssh"
EXTRA_IMAGE_FEATURES:append = " empty-root-password"
