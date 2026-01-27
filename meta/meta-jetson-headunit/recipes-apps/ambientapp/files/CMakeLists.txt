cmake_minimum_required(VERSION 3.10)
project(AmbientApp VERSION 1.0 LANGUAGES CXX)

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
find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick Network)

# CommonAPI packages
find_package(CommonAPI REQUIRED)
find_package(CommonAPI-SomeIP REQUIRED)
find_package(vsomeip3 REQUIRED)

# Generated CommonAPI sources
set(GENERATED_SOURCES
    # MediaControl (MediaApp 통신용 - Client)
    ${COMMONAPI_GEN_DIR}/someip/v1/mediacontrol/MediaControlSomeIPProxy.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/mediacontrol/MediaControlSomeIPStubAdapter.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/mediacontrol/MediaControlSomeIPDeployment.cpp

    # VehicleControl (VehicleControlECU 통신용 - Client)
    ${COMMONAPI_GEN_DIR}/someip/v1/vehiclecontrol/VehicleControlSomeIPProxy.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/vehiclecontrol/VehicleControlSomeIPStubAdapter.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/vehiclecontrol/VehicleControlSomeIPDeployment.cpp

    # AmbientControl (AmbientApp 서비스 제공 - Service)
    ${COMMONAPI_GEN_DIR}/someip/v1/ambientcontrol/AmbientControlSomeIPProxy.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/ambientcontrol/AmbientControlSomeIPStubAdapter.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/ambientcontrol/AmbientControlSomeIPDeployment.cpp
)

# Source files
set(SOURCES
    src/main.cpp
    src/ambientmanager.cpp
    src/MediaControlClient.cpp
    src/VehicleControlClient.cpp
    src/AmbientControlStubImpl.cpp
    ${GENERATED_SOURCES}
)

set(HEADERS
    src/ambientmanager.h
    src/MediaControlClient.h
    src/VehicleControlClient.h
    src/AmbientControlStubImpl.h
)

# QML resources
set(QML_RESOURCES
    qml.qrc
)

# Executable
add_executable(AmbientApp
    ${SOURCES}
    ${HEADERS}
    ${QML_RESOURCES}
)

target_include_directories(AmbientApp PRIVATE
    ${COMMONAPI_GEN_DIR}/core
    ${COMMONAPI_GEN_DIR}/someip
)

target_link_libraries(AmbientApp
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::Network
    CommonAPI
    CommonAPI-SomeIP
    vsomeip3
)

# Install target
install(TARGETS AmbientApp DESTINATION bin)
