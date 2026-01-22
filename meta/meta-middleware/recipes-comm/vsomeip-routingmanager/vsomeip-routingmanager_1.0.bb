SUMMARY = "vsomeip Routing Manager Service"
DESCRIPTION = "systemd service for vsomeip routing manager daemon on ECU2"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit systemd

SRC_URI = "file://vsomeip-routingmanager.service"

SYSTEMD_SERVICE:${PN} = "vsomeip-routingmanager.service"
SYSTEMD_AUTO_ENABLE = "enable"

RDEPENDS:${PN} = "vsomeip vsomeip-config"

do_install() {
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/vsomeip-routingmanager.service ${D}${systemd_system_unitdir}/
}

FILES:${PN} += "${systemd_system_unitdir}/vsomeip-routingmanager.service"
