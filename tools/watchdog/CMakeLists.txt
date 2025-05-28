
cmake_minimum_required(VERSION 3.28.3)
project(watchdog)

set(CMAKE_CXX_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMDLINE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Vendor/AbyssFreetype/Vendor/CmdLine")

add_executable(${PROJECT_NAME} 
    Source/main.cpp 
)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMDLINE_DIR}/Source/Public")
target_link_libraries(${PROJECT_NAME} CmdLine)