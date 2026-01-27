SUMMARY = "HU Main App Wayland Compositor"
DESCRIPTION = "Nested Wayland Compositor for Head Unit applications (Kiosk Mode)"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "qtbase qtdeclarative qtwayland"
RDEPENDS:${PN} = "qtbase qtdeclarative qtwayland-qmlplugins weston"

SRC_URI = " \
    file://CMakeLists.txt \
    file://src/ \
    file://qml/ \
    file://asset/ \
    file://qml_compositor.qrc \
    file://hu-mainapp-compositor.service \
"

S = "${WORKDIR}"

inherit cmake_qt5 systemd

EXTRA_OECMAKE = " \
    -DCMAKE_BUILD_TYPE=Release \
"

do_install:append() {
    # systemd service
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/hu-mainapp-compositor.service ${D}${systemd_system_unitdir}/
}

SYSTEMD_SERVICE:${PN} = "hu-mainapp-compositor.service"
SYSTEMD_AUTO_ENABLE = "enable"

FILES:${PN} += "${bindir}/HU_MainApp_Compositor"
