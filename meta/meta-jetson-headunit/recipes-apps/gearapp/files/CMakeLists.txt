cmake_minimum_required(VERSION 3.10)
project(GearApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Yocto environment - use COMMONAPI_GEN_DIR from recipe
if(NOT DEFINED COMMONAPI_GEN_DIR)
    message(FATAL_ERROR "COMMONAPI_GEN_DIR must be set")
endif()

message(STATUS "CommonAPI generated code directory: ${COMMONAPI_GEN_DIR}")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt packages
find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick)

# CommonAPI packages
find_package(CommonAPI REQUIRED)
find_package(CommonAPI-SomeIP REQUIRED)
find_package(vsomeip3 REQUIRED)

# Source files
set(SOURCES
    src/main.cpp
    src/gearmanager.cpp
    src/VehicleControlClient.cpp
)

set(HEADERS
    src/gearmanager.h
    src/VehicleControlClient.h
)

# CommonAPI generated sources (VehicleControl)
set(COMMONAPI_SOURCES
    ${COMMONAPI_GEN_DIR}/someip/v1/vehiclecontrol/VehicleControlSomeIPProxy.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/vehiclecontrol/VehicleControlSomeIPDeployment.cpp
)

# QML resources
set(QML_RESOURCES
    qml.qrc
)

# Executable
add_executable(GearApp
    ${SOURCES}
    ${HEADERS}
    ${COMMONAPI_SOURCES}
    ${QML_RESOURCES}
)

target_include_directories(GearApp PRIVATE
    ${COMMONAPI_GEN_DIR}/core
    ${COMMONAPI_GEN_DIR}/someip
)

target_link_libraries(GearApp
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    CommonAPI
    CommonAPI-SomeIP
    vsomeip3
)

# Install target
install(TARGETS GearApp DESTINATION bin)
