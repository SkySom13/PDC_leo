SUMMARY = "Ambient Lighting Control Application for Jetson HU"
DESCRIPTION = "Qt5/QML ambient control with vsomeip service providing AmbientControl interface"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "qtbase qtdeclarative qtquickcontrols2 vsomeip commonapi-core commonapi-someip-runtime commonapi-generated boost"
RDEPENDS:${PN} = "qtwayland qtgraphicaleffects qtquickcontrols2-qmlplugins vsomeip weston"

SRC_URI = " \
    file://CMakeLists.txt \
    file://src/ \
    file://qml/ \
    file://images/ \
    file://qml.qrc \
    file://ambientapp.service \
"

S = "${WORKDIR}"

inherit cmake_qt5 systemd

EXTRA_OECMAKE = " \
    -DCMAKE_BUILD_TYPE=Release \
    -DCOMMONAPI_GEN_DIR=${STAGING_INCDIR}/commonapi-generated \
    -DDEPLOY_PREFIX=${STAGING_DIR_HOST}${prefix} \
    -DQt5_DIR=${STAGING_LIBDIR}/cmake/Qt5 \
"

SYSTEMD_SERVICE:${PN} = "ambientapp.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install:append() {
    install -d ${D}${bindir}
    if [ -f ${B}/AmbientApp ]; then
        install -m 0755 ${B}/AmbientApp ${D}${bindir}/
    fi

    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/ambientapp.service ${D}${systemd_unitdir}/system/

    install -d ${D}${datadir}/ambientapp/qml
    if [ -d ${WORKDIR}/qml ]; then
        cp -r ${WORKDIR}/qml/* ${D}${datadir}/ambientapp/qml/ || true
    fi
}

FILES:${PN} += "${bindir}/AmbientApp ${datadir}/ambientapp/"
INSANE_SKIP:${PN} += "already-stripped"
