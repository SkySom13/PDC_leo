SUMMARY = "vSomeIP Configuration Files"
DESCRIPTION = "CommonAPI and vSomeIP configuration files for Jetson HU applications"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = " \
    file://routing_manager_ecu2.json \
"

S = "${WORKDIR}"

do_install() {
    # vsomeip routing manager configuration
    install -d ${D}${sysconfdir}/vsomeip
    install -m 0644 ${WORKDIR}/routing_manager_ecu2.json ${D}${sysconfdir}/vsomeip/

    # CommonAPI shared configuration
    install -d ${D}${datadir}/commonapi
    echo '[local]' > ${D}${datadir}/commonapi/commonapi.ini
    echo 'binding=someip' >> ${D}${datadir}/commonapi/commonapi.ini
    echo 'default=/usr/lib/libCommonAPI-SomeIP.so' >> ${D}${datadir}/commonapi/commonapi.ini
}

FILES:${PN} = " \
    ${sysconfdir}/vsomeip/* \
    ${datadir}/commonapi/* \
"

RDEPENDS:${PN} = "vsomeip commonapi-core commonapi-someip-runtime"
