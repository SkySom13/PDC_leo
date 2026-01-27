FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://weston.ini \
    file://weston.service \
"

do_install:append() {
    # Force install Jetson-specific weston.ini (override any existing)
    install -d ${D}${sysconfdir}/xdg/weston
    rm -f ${D}${sysconfdir}/xdg/weston/weston.ini
    install -m 0644 ${WORKDIR}/weston.ini ${D}${sysconfdir}/xdg/weston/weston.ini

    # Force override systemd service
    install -d ${D}${systemd_system_unitdir}
    rm -f ${D}${systemd_system_unitdir}/weston.service
    install -m 0644 ${WORKDIR}/weston.service ${D}${systemd_system_unitdir}/weston.service
}

# Ensure DRM backend is enabled (should already be from meta-tegra)
PACKAGECONFIG:append = " systemd"

FILES:${PN} += "${sysconfdir}/xdg/weston/weston.ini ${systemd_system_unitdir}/weston.service"
