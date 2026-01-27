FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

# Our custom weston.ini and weston.service for Jetson with DP-1 configuration
SRC_URI += " \
    file://weston.ini \
    file://weston.service \
"

do_install:append() {
    # Force install our custom weston.ini (override weston-init's default)
    install -D -p -m0644 ${WORKDIR}/weston.ini ${D}${sysconfdir}/xdg/weston/weston.ini
    
    # Force install our custom weston.service (override weston-init's default)
    install -D -p -m0644 ${WORKDIR}/weston.service ${D}${systemd_system_unitdir}/weston.service
}

