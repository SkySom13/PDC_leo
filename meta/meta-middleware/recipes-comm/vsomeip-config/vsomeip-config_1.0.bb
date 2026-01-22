SUMMARY = "vSomeIP Configuration Files"
DESCRIPTION = "CommonAPI and vSomeIP configuration files for Head Unit applications"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# Use external source directory
EXTERNALSRC = "/home/seame/ChangGit2/DES_Head-Unit/app"

inherit externalsrc

do_install() {
    # Create configuration directories
    install -d ${D}${sysconfdir}/vsomeip
    install -d ${D}${sysconfdir}/commonapi
    install -d ${D}${bindir}

    # Install Raspberry Pi run script (from root directory)
    install -m 0755 ${EXTERNALSRC}/../run-rpi-all.sh ${D}${bindir}/run-rpi-all.sh

    # Install routingmanagerd configuration
    [ -f ${EXTERNALSRC}/config/routing_manager_ecu2.json ] && \
        install -m 0644 ${EXTERNALSRC}/config/routing_manager_ecu2.json ${D}${sysconfdir}/vsomeip/ || true

    # Install per-application vsomeip JSON files (only if they exist)
    [ -f ${EXTERNALSRC}/GearApp/config/vsomeip_ecu2.json ] && \
        install -m 0644 ${EXTERNALSRC}/GearApp/config/vsomeip_ecu2.json ${D}${sysconfdir}/vsomeip/vsomeip_gear.json || true
    [ -f ${EXTERNALSRC}/MediaApp/vsomeip.json ] && \
        install -m 0644 ${EXTERNALSRC}/MediaApp/vsomeip.json ${D}${sysconfdir}/vsomeip/vsomeip_media.json || true
    [ -f ${EXTERNALSRC}/AmbientApp/vsomeip_ambient.json ] && \
        install -m 0644 ${EXTERNALSRC}/AmbientApp/vsomeip_ambient.json ${D}${sysconfdir}/vsomeip/ || true
    [ -f ${EXTERNALSRC}/HomeScreenApp/vsomeip_homescreen.json ] && \
        install -m 0644 ${EXTERNALSRC}/HomeScreenApp/vsomeip_homescreen.json ${D}${sysconfdir}/vsomeip/ || true
    [ -f ${EXTERNALSRC}/IC_app/vsomeip_ic.json ] && \
        install -m 0644 ${EXTERNALSRC}/IC_app/vsomeip_ic.json ${D}${sysconfdir}/vsomeip/ || true

    # Install per-application CommonAPI ini files (only if they exist)
    [ -f ${EXTERNALSRC}/GearApp/config/commonapi_ecu2.ini ] && \
        install -m 0644 ${EXTERNALSRC}/GearApp/config/commonapi_ecu2.ini ${D}${sysconfdir}/commonapi/commonapi.ini || true
    [ -f ${EXTERNALSRC}/MediaApp/commonapi_media.ini ] && \
        install -m 0644 ${EXTERNALSRC}/MediaApp/commonapi_media.ini ${D}${sysconfdir}/commonapi/ || true
    [ -f ${EXTERNALSRC}/AmbientApp/commonapi_ambient.ini ] && \
        install -m 0644 ${EXTERNALSRC}/AmbientApp/commonapi_ambient.ini ${D}${sysconfdir}/commonapi/ || true
    [ -f ${EXTERNALSRC}/HomeScreenApp/commonapi_homescreen.ini ] && \
        install -m 0644 ${EXTERNALSRC}/HomeScreenApp/commonapi_homescreen.ini ${D}${sysconfdir}/commonapi/ || true
    [ -f ${EXTERNALSRC}/IC_app/commonapi.ini ] && \
        install -m 0644 ${EXTERNALSRC}/IC_app/commonapi.ini ${D}${sysconfdir}/commonapi/commonapi_ic.ini || true
}

FILES:${PN} = " \
    ${sysconfdir}/vsomeip/* \
    ${sysconfdir}/commonapi/* \
    ${bindir}/run-rpi-all.sh \
"

RDEPENDS:${PN} = "vsomeip commonapi-core commonapi-someip-runtime bash"
