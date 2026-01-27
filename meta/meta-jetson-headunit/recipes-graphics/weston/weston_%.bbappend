FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://weston.ini \
    file://weston.service \
"

do_install:append() {
    # Install Jetson-specific weston.ini
    install -d ${D}${sysconfdir}/xdg/weston
    install -m 0644 ${WORKDIR}/weston.ini ${D}${sysconfdir}/xdg/weston/weston.ini

    # Override systemd service
    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/weston.service ${D}${systemd_system_unitdir}/weston.service
}

# Ensure DRM backend is enabled (should already be from meta-tegra)
PACKAGECONFIG:append = " systemd"

FILES:${PN} += "${sysconfdir}/xdg/weston/weston.ini ${systemd_system_unitdir}/weston.service"
