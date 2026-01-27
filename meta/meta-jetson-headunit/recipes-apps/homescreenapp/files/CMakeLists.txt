cmake_minimum_required(VERSION 3.10)
project(HomeScreenApp VERSION 1.0 LANGUAGES CXX)

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
find_package(Qt5 REQUIRED COMPONENTS Core Gui Qml Quick)

# CommonAPI packages  
find_package(CommonAPI REQUIRED)
find_package(CommonAPI-SomeIP REQUIRED)
find_package(vsomeip3 REQUIRED)

# Generated CommonAPI sources (from commonapi-generated recipe)
set(GENERATED_SOURCES_MEDIACONTROL
    ${COMMONAPI_GEN_DIR}/someip/v1/mediacontrol/MediaControlSomeIPProxy.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/mediacontrol/MediaControlSomeIPStubAdapter.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/mediacontrol/MediaControlSomeIPDeployment.cpp
)

set(GENERATED_SOURCES_AMBIENTCONTROL
    ${COMMONAPI_GEN_DIR}/someip/v1/ambientcontrol/AmbientControlSomeIPProxy.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/ambientcontrol/AmbientControlSomeIPStubAdapter.cpp
    ${COMMONAPI_GEN_DIR}/someip/v1/ambientcontrol/AmbientControlSomeIPDeployment.cpp
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

target_include_directories(HomeScreenApp PRIVATE
    ${COMMONAPI_GEN_DIR}/core
    ${COMMONAPI_GEN_DIR}/someip
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
