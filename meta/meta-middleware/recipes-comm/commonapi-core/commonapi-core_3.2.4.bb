SUMMARY = "CommonAPI C++ - IPC middleware abstraction layer"
DESCRIPTION = "CommonAPI C++ provides a language binding for the Franca IDL"
HOMEPAGE = "https://github.com/COVESA/capicxx-core-runtime"
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=815ca599c9df247a0c7f619bab123dad"

SRC_URI = "git://github.com/COVESA/capicxx-core-runtime.git;protocol=https;branch=master"
SRCREV = "${AUTOREV}"
PV = "3.2.4+git${SRCPV}"

S = "${WORKDIR}/git"

inherit cmake

EXTRA_OECMAKE = " \
    -DCMAKE_INSTALL_PREFIX=${prefix} \
"

FILES:${PN} += " \
    ${libdir}/libCommonAPI.so.* \
"

FILES:${PN}-dev += " \
    ${includedir}/CommonAPI \
    ${libdir}/cmake/CommonAPI-* \
"

BBCLASSEXTEND = "native nativesdk"
