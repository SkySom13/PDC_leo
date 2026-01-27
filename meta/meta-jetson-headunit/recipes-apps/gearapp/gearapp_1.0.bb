SUMMARY = "Gear Selection Application for Jetson HU"
DESCRIPTION = "Qt5/QML gear selector with vsomeip communication to VehicleControlECU"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "qtbase qtdeclarative qtquickcontrols2 vsomeip commonapi-core commonapi-someip-runtime commonapi-generated"
RDEPENDS:${PN} = "qtwayland qtgraphicaleffects qtquickcontrols2-qmlplugins vsomeip weston"

SRC_URI = " \
    file://CMakeLists.txt \
    file://src/ \
    file://qml/ \
    file://qml.qrc \
    file://gearapp.service \
"

S = "${WORKDIR}"

inherit cmake_qt5 systemd

EXTRA_OECMAKE = " \
    -DCMAKE_BUILD_TYPE=Release \
    -DCOMMONAPI_GEN_DIR=${STAGING_INCDIR}/commonapi-generated \
    -DDEPLOY_PREFIX=${STAGING_DIR_HOST}${prefix} \
    -DQt5_DIR=${STAGING_LIBDIR}/cmake/Qt5 \
"

SYSTEMD_SERVICE:${PN} = "gearapp.service"
SYSTEMD_AUTO_ENABLE = "enable"

do_install:append() {
    # 바이너리 설치
    install -d ${D}${bindir}
    if [ -f ${B}/GearApp ]; then
        install -m 0755 ${B}/GearApp ${D}${bindir}/
    fi

    # systemd service
    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${WORKDIR}/gearapp.service ${D}${systemd_unitdir}/system/

    # QML 리소스 (런타임에서 사용 가능하도록)
    install -d ${D}${datadir}/gearapp/qml
    if [ -d ${WORKDIR}/qml ]; then
        cp -r ${WORKDIR}/qml/* ${D}${datadir}/gearapp/qml/ || true
    fi
}

FILES:${PN} += " \
    ${bindir}/GearApp \
    ${datadir}/gearapp/ \
"

# CommonAPI 생성 코드 포함
INSANE_SKIP:${PN} += "already-stripped"
