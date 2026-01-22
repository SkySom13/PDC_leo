SUMMARY = "Jetson Orin Nano Head Unit Image"
DESCRIPTION = "Minimal Wayland/Weston image with Qt HU apps (Desktop-Shell mode)"
LICENSE = "MIT"

# 베이스 이미지
require recipes-core/images/core-image-minimal.bb

# systemd 사용
DISTRO_FEATURES:append = " systemd"
VIRTUAL-RUNTIME_init_manager = "systemd"
VIRTUAL-RUNTIME_initscripts = ""

# Wayland 활성화 (X11 제거)
DISTRO_FEATURES:remove = "x11"
DISTRO_FEATURES:append = " wayland"

# 필수 패키지 그룹
IMAGE_INSTALL:append = " \
    packagegroup-core-boot \
    packagegroup-core-full-cmdline \
"

# Weston (NVIDIA 최적화)
IMAGE_INSTALL:append = " \
    weston \
    weston-init \
"

# Qt 5.15
IMAGE_INSTALL:append = " \
    qtbase \
    qtdeclarative \
    qtquickcontrols2 \
    qtwayland \
    qtgraphicaleffects \
    qtmultimedia \
"

# 네트워크
IMAGE_INSTALL:append = " \
    iproute2 \
    iputils \
"

# 개발 도구
IMAGE_INSTALL:append = " \
    openssh \
    htop \
    nano \
"

# 루트파일시스템 크기
IMAGE_ROOTFS_SIZE ?= "2097152"

# 타겟 포맷
IMAGE_FSTYPES = "tegraflash tar.gz"
