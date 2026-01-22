SUMMARY = "vsomeip - An implementation of Scalable service-Oriented MiddlewarE over IP"
DESCRIPTION = "vsomeip is an implementation of the SOME/IP protocol"
HOMEPAGE = "https://github.com/COVESA/vsomeip"
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=9741c346eef56131163e13b9db1241b3"

DEPENDS = "boost"

SRC_URI = "git://github.com/COVESA/vsomeip.git;protocol=https;branch=master"
SRCREV = "e89240c7d5d506505326987b6a2f848658230641"
PV = "3.5.8"

S = "${WORKDIR}/git"

inherit cmake

EXTRA_OECMAKE = " \
    -DCMAKE_INSTALL_PREFIX=${prefix} \
    -DENABLE_SIGNAL_HANDLING=1 \
    -DBUILD_EXAMPLES=ON \
"

do_install:append() {
    # Install routingmanagerd from examples
    install -d ${D}${bindir}
    if [ -f ${B}/examples/routingmanagerd/routingmanagerd ]; then
        install -m 0755 ${B}/examples/routingmanagerd/routingmanagerd ${D}${bindir}/
    fi
}

FILES:${PN} += " \
    ${libdir}/libvsomeip3*.so.* \
    ${bindir}/routingmanagerd \
"

# Skip QA check for unshipped config files (example/template files only)
INSANE_SKIP:${PN} += "installed-vs-shipped"

FILES:${PN}-dev += " \
    ${includedir}/vsomeip \
    ${libdir}/cmake/vsomeip3 \
    ${libdir}/libvsomeip3*.so \
"

BBCLASSEXTEND = "native nativesdk"
