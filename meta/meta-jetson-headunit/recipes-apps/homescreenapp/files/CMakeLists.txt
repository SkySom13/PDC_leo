cmake_minimum_required(VERSION 3.10)
project(HomeScreenApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ════════════════════════════════════════════════════════════
# 배포 환경별 경로 설정 (환경변수 우선)
# ════════════════════════════════════════════════════════════
if(DEFINED ENV{DEPLOY_PREFIX})
    set(INSTALL_PREFIX $ENV{DEPLOY_PREFIX})
    message(STATUS "Using DEPLOY_PREFIX from environment: ${INSTALL_PREFIX}")
else()
    # 기본값: 상대 경로 기준 (app/HomeScreenApp 기준으로 ../../install_folder)
    set(INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/../../install_folder")
    message(STATUS "Using default DEPLOY_PREFIX: ${INSTALL_PREFIX}")
endif()

# RPATH 설정 (vsomeip, CommonAPI 라이브러리 경로)
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${INSTALL_PREFIX}")
set(CMAKE_INSTALL_RPATH "${INSTALL_PREFIX}/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt packages
find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick)

# CommonAPI packages
find_package(CommonAPI REQUIRED)
find_package(CommonAPI-SomeIP REQUIRED)
find_package(vsomeip3 REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../commonapi/generated/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../../commonapi/generated/someip
)

# Generated CommonAPI sources
set(GENERATED_SOURCES_MEDIACONTROL
    ${CMAKE_CURRENT_SOURCE_DIR}/../../commonapi/generated/someip/v1/mediacontrol/MediaControlSomeIPProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../commonapi/generated/someip/v1/mediacontrol/MediaControlSomeIPStubAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../commonapi/generated/someip/v1/mediacontrol/MediaControlSomeIPDeployment.cpp
)

set(GENERATED_SOURCES_AMBIENTCONTROL
    ${CMAKE_CURRENT_SOURCE_DIR}/../../commonapi/generated/someip/v1/ambientcontrol/AmbientControlSomeIPProxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../commonapi/generated/someip/v1/ambientcontrol/AmbientControlSomeIPStubAdapter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../commonapi/generated/someip/v1/ambientcontrol/AmbientControlSomeIPDeployment.cpp
)

# HomeScreenApp executable
add_executable(HomeScreenApp
    src/main.cpp
    src/homescreenmanager.cpp
    src/homescreenmanager.h
    src/MediaControlClient.cpp
    src/MediaControlClient.h
    src/AmbientControlClient.cpp
    src/AmbientControlClient.h
    ${GENERATED_SOURCES_MEDIACONTROL}
    ${GENERATED_SOURCES_AMBIENTCONTROL}
    qml.qrc
)

target_link_libraries(HomeScreenApp
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    CommonAPI
    CommonAPI-SomeIP
    vsomeip3
)

# Install target
install(TARGETS HomeScreenApp DESTINATION bin)
