cmake_minimum_required(VERSION 3.10)
project(MediaApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Yocto environment - use COMMONAPI_GEN_DIR from recipe
if(NOT DEFINED COMMONAPI_GEN_DIR)
    message(FATAL_ERROR "COMMONAPI_GEN_DIR must be set")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt packages
find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick Multimedia)

# CommonAPI packages
find_package(CommonAPI REQUIRED)
find_package(CommonAPI-SomeIP REQUIRED)
find_package(vsomeip3 REQUIRED)

# Generated CommonAPI sources
set(GENERATED_SOURCES
    # MediaControl (MediaApp provides this service)
    ${COMMONAPI_GEN_DIR}/someip/v1/mediacontrol/MediaControlSomeIPProxy.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/mediacontrol/MediaControlSomeIPStubAdapter.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/mediacontrol/MediaControlSomeIPDeployment.cpp

    # AmbientControl (MediaApp subscribes to this service)
    ${COMMONAPI_GEN_DIR}/someip/v1/ambientcontrol/AmbientControlSomeIPProxy.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/ambientcontrol/AmbientControlSomeIPStubAdapter.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/ambientcontrol/AmbientControlSomeIPDeployment.cpp
)

# Source files
set(SOURCES
    src/main.cpp
    src/mediamanager.cpp
    src/MediaControlStubImpl.cpp
    src/AmbientControlClient.cpp
    src/ambientthememanager.cpp
    ${GENERATED_SOURCES}
)

set(HEADERS
    src/mediamanager.h
    src/MediaControlStubImpl.h
    src/AmbientControlClient.h
    src/ambientthememanager.h
)

# QML resources
set(QML_RESOURCES
    qml.qrc
)

# Executable
add_executable(MediaApp
    ${SOURCES}
    ${HEADERS}
    ${QML_RESOURCES}
)

target_include_directories(MediaApp PRIVATE
    ${COMMONAPI_GEN_DIR}/core
    ${COMMONAPI_GEN_DIR}/someip
)

target_link_libraries(MediaApp
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Multimedia
    CommonAPI
    CommonAPI-SomeIP
    vsomeip3
)

# Install target
install(TARGETS MediaApp DESTINATION bin)
