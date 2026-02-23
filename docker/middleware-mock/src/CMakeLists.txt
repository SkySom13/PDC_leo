cmake_minimum_required(VERSION 3.10)
project(VehicleMockServer)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CommonAPI/SomeIP headers from install_folder
include_directories(
    ${CMAKE_SOURCE_DIR}/app/commonapi/generated
    /usr/local/include
)

find_library(COMMONAPI_LIB CommonAPI PATHS /usr/local/lib)
find_library(COMMONAPI_SOMEIP_LIB CommonAPI-SomeIP PATHS /usr/local/lib)
find_library(VSOMEIP_LIB vsomeip3 PATHS /usr/local/lib)

if(NOT COMMONAPI_LIB OR NOT COMMONAPI_SOMEIP_LIB OR NOT VSOMEIP_LIB)
    message(FATAL_ERROR "Missing CommonAPI/vsomeip libs")
endif()

add_executable(vehicle_mock_server main.cpp)

target_link_libraries(vehicle_mock_server
    ${COMMONAPI_LIB}
    ${COMMONAPI_SOMEIP_LIB}
    ${VSOMEIP_LIB}
)
