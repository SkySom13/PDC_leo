SUMMARY = "CommonAPI Generated Code for HU/IC Apps"
DESCRIPTION = "Auto-generated CommonAPI proxy/stub code for MediaControl, AmbientControl, VehicleControl"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS = "commonapi-core commonapi-someip-runtime vsomeip"

SRC_URI = " \
    file://core/ \
    file://someip/ \
"

S = "${WORKDIR}"

# Header-only 패키지 (일부 cpp 포함)
inherit cmake

do_configure[noexec] = "1"
do_compile[noexec] = "1"

do_install() {
    # Include 디렉토리 설치
    install -d ${D}${includedir}/commonapi-generated/core
    install -d ${D}${includedir}/commonapi-generated/someip

    # Core headers
    cp -r ${S}/core/* ${D}${includedir}/commonapi-generated/core/

    # SomeIP headers and sources
    cp -r ${S}/someip/* ${D}${includedir}/commonapi-generated/someip/
}

FILES:${PN}-dev = "${includedir}/commonapi-generated/"
ALLOW_EMPTY:${PN} = "1"
