SUMMARY = "VehicleControl Mock Service for Testing"
DESCRIPTION = "vsomeip routing manager and mock vehicle control service for testing without hardware"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = " \
    qtbase \
    vsomeip \
    commonapi-core \
    commonapi-someip-runtime \
"

RDEPENDS:${PN} = " \
    qtbase \
    vsomeip \
    commonapi-core \
    commonapi-someip-runtime \
"

# Use external source directory for local development
SRC_URI = "file://vehiclecontrolmock.service"
EXTERNALSRC = "/home/seame/ChangGit2/DES_Head-Unit/app/VehicleControlMock"
EXTERNALSRC_BUILD = "${EXTERNALSRC}/build"

inherit cmake_qt5 systemd externalsrc

# CommonAPI generated code path
EXTRA_OECMAKE += "-DCOMMONAPI_GEN_DIR=/home/seame/ChangGit2/DES_Head-Unit/commonapi/generated"
EXTRA_OECMAKE += "-DQT_QMAKE_EXECUTABLE=${STAGING_BINDIR_NATIVE}/qmake"

# Systemd service configuration
SYSTEMD_SERVICE:${PN} = "vehiclecontrolmock.service"
SYSTEMD_AUTO_ENABLE = "enable"

# Install paths
FILES:${PN} += " \
    ${bindir}/VehicleControlMock \
    ${sysconfdir}/vsomeip/vsomeip_mock.json \
    ${systemd_system_unitdir}/vehiclecontrolmock.service \
"

do_install:append() {
    # Install binary
    install -d ${D}${bindir}
    install -m 0755 ${B}/VehicleControlMock ${D}${bindir}/

    # Install vsomeip configuration
    install -d ${D}${sysconfdir}/vsomeip
    install -m 0644 ${EXTERNALSRC}/config/vsomeip_mock.json ${D}${sysconfdir}/vsomeip/

    # Install systemd service
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/vehiclecontrolmock.service ${D}${systemd_system_unitdir}/
}

# This is a test/mock service, not for production
PACKAGE_ARCH = "${MACHINE_ARCH}"
