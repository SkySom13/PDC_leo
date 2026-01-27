SUMMARY = "Media Player Application for Jetson HU"
DESCRIPTION = "Qt5/QML media player with vsomeip service providing MediaControl interface"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "qtbase qtdeclarative qtmultimedia qtquickcontrols2 vsomeip commonapi-core commonapi-someip-runtime commonapi-generated boost"
RDEPENDS:${PN} = "qtwayland qtmultimedia qtgraphicaleffects qtquickcontrols2-qmlplugins qtmultimedia-qmlplugins vsomeip weston"

SRC_URI = " \
    file://CMakeLists.txt \
    file://src/ \
    file://qml/ \
    file://asset/ \
    file://images/ \
    file://qml.qrc \
    file://mediaapp.service \
"

S = "${WORKDIR}"

inherit cmake_qt5 systemd

EXTRA_OECMAKE = " \
    -DCMAKE_BUILD_TYPE=Release \
    -DCOMMONAPI_GEN_DIR=${STAGING_INCDIR}/commonapi-generated \
    -DDEPLOY_PREFIX=${STAGING_DIR_HOST}${prefix} \
    -DQt5_DIR=${STAGING_LIBDIR}/cmake/Qt5 \
"

SYSTEMD_SERVICE:${PN} = "mediaapp.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install:append() {
    install -d ${D}${bindir}
    if [ -f ${B}/MediaApp ]; then
        install -m 0755 ${B}/MediaApp ${D}${bindir}/
    fi

    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/mediaapp.service ${D}${systemd_unitdir}/system/

    install -d ${D}${datadir}/mediaapp/qml
    if [ -d ${WORKDIR}/qml ]; then
        cp -r ${WORKDIR}/qml/* ${D}${datadir}/mediaapp/qml/ || true
    fi
}

FILES:${PN} += "${bindir}/MediaApp ${datadir}/mediaapp/"
INSANE_SKIP:${PN} += "already-stripped"
