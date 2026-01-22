SUMMARY = "CommonAPI C++ SOME/IP Runtime"
DESCRIPTION = "CommonAPI SOME/IP provides SOME/IP binding for CommonAPI"
HOMEPAGE = "https://github.com/COVESA/capicxx-someip-runtime"
LICENSE = "MPL-2.0"
LIC_FILES_CHKSUM = "file://LICENSE;md5=815ca599c9df247a0c7f619bab123dad"

DEPENDS = "commonapi-core vsomeip boost"

SRC_URI = "git://github.com/COVESA/capicxx-someip-runtime.git;protocol=https;branch=master"
SRCREV = "${AUTOREV}"
PV = "3.2.4+git${SRCPV}"

S = "${WORKDIR}/git"

inherit cmake

EXTRA_OECMAKE = " \
    -DCMAKE_INSTALL_PREFIX=${prefix} \
    -DUSE_INSTALLED_COMMONAPI=ON \
"

FILES:${PN} += " \
    ${libdir}/libCommonAPI-SomeIP.so.* \
"

FILES:${PN}-dev += " \
    ${includedir}/CommonAPI \
    ${libdir}/cmake/CommonAPI-SomeIP* \
"

BBCLASSEXTEND = "native nativesdk"
