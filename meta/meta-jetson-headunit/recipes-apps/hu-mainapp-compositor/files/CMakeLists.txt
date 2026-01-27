cmake_minimum_required(VERSION 3.16)

project(HU_MainApp_Compositor VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Wayland Compositor Only - No Manager Dependencies
# HU_MainApp은 Wayland Compositor 역할만 수행
# 각 앱(GearApp, MediaApp, AmbientApp)은 독립 프로세스로 실행

# Qt packages - GUI + WaylandCompositor
find_package(Qt5 REQUIRED COMPONENTS Core Quick Qml WaylandCompositor)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Source files - Compositor Only
set(SOURCES
    src/main_compositor.cpp
)

# QML resources
set(QML_RESOURCES
    qml_compositor.qrc
)

# Executable
add_executable(HU_MainApp_Compositor
    ${SOURCES}
    ${QML_RESOURCES}
)

target_link_libraries(HU_MainApp_Compositor
    Qt5::Core
    Qt5::Quick
    Qt5::Qml
    Qt5::WaylandCompositor
)

# Install target
install(TARGETS HU_MainApp_Compositor DESTINATION bin)
