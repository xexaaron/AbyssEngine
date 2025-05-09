
cmake_minimum_required(VERSION 3.28.3)
project(watchdog)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)

set(CMDLINE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Vendor/CmdLine/Source")

add_executable(${PROJECT_NAME} 
    Source/main.cpp 
    ${CMDLINE_DIR}/Private/CmdLine.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMDLINE_DIR}/Public")